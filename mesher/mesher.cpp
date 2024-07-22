#include "mesher.h"
#include "buffer.h"
#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "cube.h"
#include "forward.h"
#include "scene/resources/mesh.h"
#include "servers/rendering_server.h"
#include "voxel_block.h"
#include "voxel_block_library.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <glm/fwd.hpp>
#include <unordered_map>
#include <vector>

namespace pgvoxel {

static void add_data_to_surface(const glm::vec3 &block_pos, Model::BakedData::Surface::Data &surface_data, const Model::BakedData::Surface::Data &new_data, Side side) {
	const size_t vertex_count = new_data.positions.size();

	const size_t index_count = surface_data.indices.size();
	const size_t index_offset = surface_data.positions.size();
	// 添加 indices
	{
		surface_data.indices.reserve(surface_data.indices.size() + index_count);
		for (unsigned int i = 0; i < index_count; ++i) {
			surface_data.indices.push_back(index_offset + surface_data.indices[i]);
		}
	}

	// 添加 positions
	{
		surface_data.positions.reserve(surface_data.positions.size() + vertex_count);
		for (unsigned int i = 0; i < vertex_count; ++i) {
			surface_data.positions.push_back(surface_data.positions[i] + block_pos);
		}
	}

	// 添加 uvs
	{
		const size_t append_index = surface_data.uvs.size();
		surface_data.uvs.resize(append_index + vertex_count);
		memcpy(surface_data.uvs.data() + append_index, surface_data.uvs.data(), vertex_count * sizeof(glm::vec2));
	}

	// 添加 tangents
	if (surface_data.tangents.size() > 0) {
		const size_t append_index = surface_data.tangents.size();
		surface_data.tangents.resize(append_index + vertex_count * 4);
		memcpy(surface_data.tangents.data() + append_index, surface_data.tangents.data(), (vertex_count * 4) * sizeof(float));
	}

	// 添加 normals
	if (side == SIDE_COUNT) {
		// 在内部的 Normal 需要从新数据中读取
		const size_t append_index = surface_data.normals.size();
		surface_data.normals.resize(append_index + vertex_count);
		memcpy(surface_data.normals.data() + append_index, surface_data.normals.data(), vertex_count * sizeof(glm::vec3));
	} else {
		// 在面上的 Normal 可以直接查表获得
		const auto normal = kSideNormalLut[side];
		const size_t append_index = surface_data.normals.size();
		surface_data.normals.resize(append_index + vertex_count);
		auto offset = surface_data.normals.data() + append_index;
		for (unsigned int i = 0; i < vertex_count; ++i) {
			offset[i] = normal;
		}
	}
}

static void add_data_to_collision(const glm::vec3 &block_pos, std::vector<glm::vec3> &positions, std::vector<int> &indices, const Model::BakedData::Surface::Data &new_data) {
	const size_t vertex_count = new_data.positions.size();
	const size_t index_offset = positions.size();
	const size_t index_count = new_data.indices.size();

	positions.reserve(positions.size() + vertex_count);
	for (const auto &pos : new_data.positions) {
		positions.push_back(pos + block_pos);
	}

	indices.reserve(indices.size() + index_count);
	for (auto index : new_data.indices) {
		indices.push_back(index_offset + index);
	}
}

bool VoxelMesher::is_face_visible(const Model &target_block, const uint32_t neighbor_block_id, const Side side) const {
	if (library_->has_block(neighbor_block_id)) {
		// 如果相邻方块在lib中不存在，这里不进行报错而是简单的跳过，因为在处理到相邻方块时已经报错过了，再报错就重复了
		const Model &neighbor_block = library_->get_block(neighbor_block_id)->get_model();
		if (neighbor_block.get_baked_data().sides_mask == 0 || !neighbor_block.culls_neighbors || (neighbor_block.transparency_index > target_block.transparency_index)) {
			// 如果相邻方块是空方块，或者相邻方块不遮挡当前方块，则当前方块可见
			return true;
		} else {
			const auto ai = target_block.get_baked_data().side_pattern[side];
			const auto bi = neighbor_block.get_baked_data().side_pattern[kOppositeSideLut[side]];
			// 当前面被相邻面包含时，(ai & bi) == ai，当前面不可见
			return (ai & bi) != ai;
		}
	}
	// 当相邻方块不存在时，总是显示为空方块，因此当前面总是可见的
	return true;
}

Ref<ArrayMesh> VoxelMesher::build(const Buffer &data) {
	Coord min{ 0, 0, 0 };
	Coord max(data.getWidth(), data.getHeight(), data.getDepth());
	if (padding_) {
		min = Coord{ 1, 1, 1 };
		max = Coord{ max.x - 1, max.y - 1, max.z - 1 };
	}

	// 每个 Surface 材质不同，因此以 Material 的 RID 作为索引分开存
	std::unordered_map<uint64_t, Model::BakedData::Surface::Data> surface_datas;

	// 碰撞 Mesh 没有不同材质，直接存一起
	std::vector<glm::vec3> collision_positions;
	std::vector<int> collision_indices;

	for (CoordAxis z = min.z; z < (unsigned int)max.z; ++z) {
		for (CoordAxis x = min.x; x < (unsigned int)max.x; ++x) {
			for (CoordAxis y = min.y; y < (unsigned int)max.y; ++y) {
				const glm::vec3 block_pos = padding_ ? Coord{ x - 1, y - 1, z - 1 } : Coord{ x, y, z };
				const VoxelData block_id = data.getVoxel(block_pos);

				if (block_id == 0) {
					continue;
				}

				ERR_CONTINUE(!library_->has_block(block_id));

				const Model &model = library_->get_block(block_id)->get_model();
				const Model::BakedData &baked_data = model.get_baked_data();

				if (baked_data.sides_mask == 0) {
					// 整个方块都是空的
					continue;
				}
				for (size_t surface_index = 0; surface_index < baked_data.surfaces.size(); ++surface_index) {
					const uint64_t material_rid = baked_data.surfaces[surface_index].material->get_rid().get_id();
					Model::BakedData::Surface::Data &surface_data = surface_datas[material_rid];

					// 先处理六个面上的内容
					for (unsigned int side = 0; side < SIDE_COUNT; ++side) {
						if ((baked_data.sides_mask & (1 << side)) == 0) {
							continue;
						}

						// 检测是否被相邻的面遮挡
						const auto neighbor_block_pos = block_pos + kSideNormalLut[side];
						// 如果没padding，需要判断 neighbor block pos 是否越界
						if (!padding_ && (neighbor_block_pos.x < min.x || neighbor_block_pos.y < min.y || neighbor_block_pos.z < min.z || neighbor_block_pos.x >= max.x || neighbor_block_pos.y >= max.y || neighbor_block_pos.z >= max.z)) {
							continue;
						}
						const VoxelData neighbor_voxel_id = data.getVoxel(neighbor_block_pos);
						if (!is_face_visible(model, neighbor_voxel_id, Side(side))) {
							continue;
						}

						const Model::BakedData::Surface::Data &new_surface_data = baked_data.surfaces[surface_index].side_data[side];
						add_data_to_surface(block_pos, surface_data, new_surface_data, Side(side));
						if (model.collision_enabled) {
							// 如果启用了碰撞，还要添加碰撞相关的顶点
							add_data_to_collision(block_pos, collision_positions, collision_indices, new_surface_data);
						}
					}

					if ((baked_data.sides_mask & (1 << SIDE_COUNT)) == 0) {
						// 内部是空的
						continue;
					}
					const Model::BakedData::Surface::Data &new_surface_data = baked_data.surfaces[surface_index].inside_data;
					add_data_to_surface(block_pos, surface_data, new_surface_data, SIDE_COUNT);
					if (model.collision_enabled) {
						add_data_to_collision(block_pos, collision_positions, collision_indices, new_surface_data);
					}
				}
			}
		}
	}

	Ref<ArrayMesh> mesh;
	mesh.instantiate();
	int i = 0;
	for (const auto &[material_rid, surface_data] : surface_datas) {
		Array arrays;
		arrays.resize(Mesh::ARRAY_MAX);
		arrays[Mesh::ARRAY_VERTEX] = PackedVector3Array();
		arrays[Mesh::ARRAY_TEX_UV] = PackedVector2Array();
		arrays[Mesh::ARRAY_NORMAL] = PackedVector3Array();
		arrays[Mesh::ARRAY_INDEX] = PackedInt32Array();
		arrays[Mesh::ARRAY_TANGENT] = PackedFloat32Array();

		mesh->add_surface_from_arrays(ArrayMesh::PRIMITIVE_TRIANGLES, arrays);
		RenderingServer::get_singleton()->mesh_surface_set_material(mesh->get_rid(), i, RID::from_uint64(material_rid));
	}

	return mesh;
}

Ref<ArrayMesh> VoxelMesher::gds_build(Ref<VoxelBuffer> data) {
	return build(data->data());
}

void VoxelMesher::_bind_methods() {
	ClassDB::bind_method(D_METHOD("build", "buffer"), &VoxelMesher::gds_build);

	ClassDB::bind_method(D_METHOD("set_library", "library"), &VoxelMesher::set_library);
	ClassDB::bind_method(D_METHOD("get_library"), &VoxelMesher::get_library);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "library", PROPERTY_HINT_RESOURCE_TYPE, "VoxelBlockLibrary"),
			"set_library", "get_library");

	ClassDB::bind_method(D_METHOD("set_padding", "padding"), &VoxelMesher::set_padding);
	ClassDB::bind_method(D_METHOD("get_padding"), &VoxelMesher::get_padding);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "padding"), "set_padding", "get_padding");
}

} //namespace pgvoxel
