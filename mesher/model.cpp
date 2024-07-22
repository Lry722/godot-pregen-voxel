#include "model.h"
#include "core/error/error_macros.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "cube.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <unordered_map>

namespace pgvoxel {

static uint8_t get_sides(const glm::vec3 &pos, const float tolerance) {
	uint8_t mask = 0;
	mask |= Math::is_equal_approx(pos.x, 0.f, tolerance) << SIDE_NEGATIVE_X;
	mask |= Math::is_equal_approx(pos.x, 1.f, tolerance) << SIDE_POSITIVE_X;
	mask |= Math::is_equal_approx(pos.y, 0.f, tolerance) << SIDE_NEGATIVE_Y;
	mask |= Math::is_equal_approx(pos.y, 1.f, tolerance) << SIDE_POSITIVE_Y;
	mask |= Math::is_equal_approx(pos.z, 0.f, tolerance) << SIDE_NEGATIVE_Z;
	mask |= Math::is_equal_approx(pos.z, 1.f, tolerance) << SIDE_POSITIVE_Z;
	return mask;
}

static Side get_triangle_side(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const float tolerance) {
	const uint8_t m = get_sides(a, tolerance) & get_sides(b, tolerance) & get_sides(c, tolerance);
	if (m == 0) {
		return SIDE_COUNT;
	}
	for (unsigned int side = 0; side < SIDE_COUNT; ++side) {
		if (m == (1 << side)) {
			return (Side)side;
		}
	}
}

static glm::vec2 to_vec2(const Vector2 &v) {
	return glm::vec2(v.x, v.y);
}

static glm::vec3 to_vec3(const Vector3 &v) {
	return glm::vec3(v.x, v.y, v.z);
}

static bool validate_indices(const auto &indices, int vertex_count) {
	ERR_FAIL_COND_V(vertex_count < 0, false);
	for (const int index : indices) {
		ERR_FAIL_COND_V_MSG(index < 0 || index >= vertex_count, false, vformat("Invalid index found in mesh indices. Maximum is {}, found {}", vertex_count - 1, index));
	}
	return true;
}

static bool pos_in_triangle(const glm::vec2 &P, const glm::vec2 &A, const glm::vec2 &B, const glm::vec2 &C) {
	const glm::vec2 AB = B - A;
	const glm::vec2 BC = C - B;
	const glm::vec2 CA = A - C;
	const glm::vec2 AP = P - A;
	const glm::vec2 BP = P - B;
	const glm::vec2 CP = P - C;
	const float cross1 = AB.x * AP.y - AB.y * AP.x;
	const float cross2 = BC.x * BP.y - BC.y * BP.x;
	const float cross3 = CA.x * CP.y - CA.y * CP.x;
	return (cross1 >= 0.0f && cross2 >= 0.0f && cross3 >= 0.0f) ||
			(cross1 <= 0.0f && cross2 <= 0.0f && cross3 <= 0.0f);
}

void Model::bake() {
	baked_data_.clear();

	ERR_FAIL_COND(!mesh_.is_valid());
	print_verbose(vformat("Mesh {0} start bake.", mesh_->get_rid()));

	std::vector<Ref<Material>> materials;
	std::vector<Array> surfaces;

	for (int i = 0; i < mesh_->get_surface_count(); ++i) {
		surfaces.push_back(mesh_->surface_get_arrays(i));
		materials.push_back(mesh_->surface_get_material(i));
	}

	for (unsigned int surface_index = 0; surface_index < surfaces.size(); ++surface_index) {
		print_verbose(vformat("Mesh {0} baking surface {1}.", mesh_->get_rid(), surface_index));
		const Array &arrays = surfaces[surface_index];

		ERR_CONTINUE(arrays.size() == 0);

		PackedInt32Array indices = arrays[Mesh::ARRAY_INDEX];
		ERR_CONTINUE_MSG(indices.size() == 0, "Mesh surface is empty or is missing an index buffer.");
		ERR_CONTINUE_MSG(indices.size() % 3 != 0,
				String("Mesh surface has an invalid number of indices. "
					   "Expected multiple of 3 (for triangles), found {0}")
						.format(varray(indices.size())));

		PackedVector3Array positions = arrays[Mesh::ARRAY_VERTEX];
		PackedVector3Array normals = arrays[Mesh::ARRAY_NORMAL];
		PackedVector2Array uvs = arrays[Mesh::ARRAY_TEX_UV];
		PackedFloat32Array tangents = arrays[Mesh::ARRAY_TANGENT];

		// Godot actually allows to create an ArrayMesh with invalid indices.
		// We require valid indices for baking, so we have to check it.
		if (!validate_indices(indices, positions.size())) {
			continue;
		}

		ERR_FAIL_COND_MSG(normals.size() == 0, "The mesh is missing normals, this is not supported.");
		ERR_FAIL_COND_MSG(positions.size() != normals.size(), "The number of normals and vertices do not match.");

		if (uvs.size() == 0) {
			// TODO Properly generate UVs if there arent any
			uvs = PackedVector2Array();
			uvs.resize(positions.size());
		}

		// Separate triangles belonging to faces of the cube
		BakedData::Surface &surface = baked_data_.surfaces[surface_index];
		Ref<Material> material = materials[surface_index];
		// Note, an empty material counts as "The default material".

		// 将面上的旧顶点的序号映射到新顶点的序号
		std::array<std::unordered_map<int, int>, SIDE_COUNT> sides_added_indices;
		// 将内部的旧顶点的序号映射到新顶点的序号
		std::unordered_map<int, int> added_regular_indices;

		// 每次读取三角形的三个顶点
		std::array<glm::vec3, 3> tri_positions{};
		for (int i = 0; i < indices.size(); i += 3) {
			tri_positions[0] = to_vec3(positions[indices[i]]);
			tri_positions[1] = to_vec3(positions[indices[i + 1]]);
			tri_positions[2] = to_vec3(positions[indices[i + 2]]);
			Side side = get_triangle_side(tri_positions[0], tri_positions[1], tri_positions[2], side_vertex_tolerance);

			if (side != SIDE_COUNT) {
				// 三角形在面上

				// 原本的 Mesh 会被分为六个面 + 内部顶点，每个面上的顶点数据都是独立的
				// 因此每个顶点的序号和原 Mesh 的都不一样，需要计算新的序号
				int new_index = surface.side_data[side].positions.size();

				for (int j = 0; j < 3; ++j) {
					// 先获取原 Mesh 上的顶点序号
					const int src_index = indices[i + j];

					// 查找旧顶点的新序号
					std::unordered_map<int, int> &added_indices = sides_added_indices[side];
					const auto existing_dst_index_it = added_indices.find(src_index);
					if (existing_dst_index_it == added_indices.end()) {
						// 未查找到，说明未添加过，则添加新顶点
						surface.side_data[side].indices.push_back(new_index);
						surface.side_data[side].positions.push_back(tri_positions[j]);
						surface.side_data[side].uvs.push_back(to_vec2(uvs[src_index]));

						// i is the first vertex of each triangle which increments by steps of 3.
						// There are 4 floats per tangent.
						int ti = indices[i + j] * 4;
						surface.side_data[side].tangents.push_back(tangents[ti]);
						surface.side_data[side].tangents.push_back(tangents[ti + 1]);
						surface.side_data[side].tangents.push_back(tangents[ti + 2]);
						surface.side_data[side].tangents.push_back(tangents[ti + 3]);

						added_indices.insert({ src_index, new_index });
						++new_index;

					} else {
						// 若查找到了，则直接使用新序号
						surface.side_data[side].indices.push_back(existing_dst_index_it->second);
					}
				}
				// 标记该面有数据
				baked_data_.sides_mask |= (1 << side);
				// 栅格化三角形到 side pattern 中
				// 首先要把三维的三脚面数据拍平到所处的平面上
				std::array<glm::vec2, 3> tri_positions_2d{};
				switch (side) {
					case SIDE_LEFT:
					case SIDE_RIGHT:
						tri_positions_2d[0] = glm::vec2(tri_positions[0].y, tri_positions[0].z);
						tri_positions_2d[1] = glm::vec2(tri_positions[1].y, tri_positions[1].z);
						tri_positions_2d[2] = glm::vec2(tri_positions[2].y, tri_positions[2].z);
						break;
					case SIDE_BOTTOM:
					case SIDE_TOP:
						tri_positions_2d[0] = glm::vec2(tri_positions[0].x, tri_positions[0].z);
						tri_positions_2d[1] = glm::vec2(tri_positions[1].x, tri_positions[1].z);
						tri_positions_2d[2] = glm::vec2(tri_positions[2].x, tri_positions[2].z);
						break;
					case SIDE_BACK:
					case SIDE_FRONT:
						tri_positions_2d[0] = glm::vec2(tri_positions[0].x, tri_positions[0].y);
						tri_positions_2d[1] = glm::vec2(tri_positions[1].x, tri_positions[1].y);
						tri_positions_2d[2] = glm::vec2(tri_positions[2].x, tri_positions[2].y);
					default:
						ERR_PRINT("Unexpected side.");
				}
				// 然后遍历每个点，判断是否在三角形内
				auto &pattern = baked_data_.side_pattern[side];
				unsigned cur_bit = 0;
				for (unsigned x = 0; x < BakedData::PATTERN_WIDTH; ++x) {
					for (unsigned y = 0; y < BakedData::PATTERN_WIDTH; ++y) {
						// 取每个格子的中心点
						const glm::vec2 P = (glm::vec2(x, y) + glm::vec2(0.5f)) / glm::vec2(BakedData::PATTERN_WIDTH);
						if (pos_in_triangle(P, tri_positions_2d[0], tri_positions_2d[1], tri_positions_2d[2])) {
							pattern.set(cur_bit, true);
						}
						++cur_bit;
					}
				}
			} else {
				// 三角形不在面上
				int new_index = surface.inside_data.positions.size();

				for (int j = 0; j < 3; ++j) {
					const int src_index = indices[i + j];
					const auto existing_dst_index_it = added_regular_indices.find(src_index);

					if (existing_dst_index_it == added_regular_indices.end()) {
						surface.inside_data.indices.push_back(new_index);
						surface.inside_data.positions.push_back(tri_positions[j]);
						// 添加内部点和添加面上的点的代码差不多，这里是唯一不同的一行
						surface.inside_data.normals.push_back(to_vec3(normals[src_index]));
						surface.inside_data.uvs.push_back(to_vec2(uvs[src_index]));

						int ti = indices[i + j] * 4;
						surface.inside_data.tangents.push_back(tangents[ti]);
						surface.inside_data.tangents.push_back(tangents[ti + 1]);
						surface.inside_data.tangents.push_back(tangents[ti + 2]);
						surface.inside_data.tangents.push_back(tangents[ti + 3]);

						added_regular_indices.insert({ src_index, new_index });
						++new_index;
					} else {
						surface.inside_data.indices.push_back(existing_dst_index_it->second);
					}
				}
				// 标记内部有数据
				baked_data_.sides_mask |= (1 << SIDE_COUNT);
			}
		}
	}

	baked_ = true;
	print_verbose(vformat("Mesh {0} finish bake.", mesh_->get_rid()));
}

} //namespace pgvoxel
