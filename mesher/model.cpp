#include "model.h"
#include "core/error/error_macros.h"
#include "cube.h"
#include <glm/ext/vector_float3.hpp>
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

static SideAxis get_triangle_side(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const float tolerance) {
	const uint8_t m = get_sides(a, tolerance) & get_sides(b, tolerance) & get_sides(c, tolerance);
	if (m == 0) {
		return SIDE_AXIS_COUNT;
	}
	for (unsigned int side = 0; side < SIDE_COUNT; ++side) {
		if (m == (1 << side)) {
			return (SideAxis)side;
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

void Model::bake() {
	std::vector<Ref<Material>> materials;
	std::vector<Array> surfaces;

	for (unsigned int i = 0; i < mesh_->get_surface_count(); ++i) {
		surfaces.push_back(mesh_->surface_get_arrays(i));
		materials.push_back(mesh_->surface_get_material(i));
	}

	for (unsigned int surface_index = 0; surface_index < surfaces.size(); ++surface_index) {
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

		ERR_CONTINUE_MSG(positions.size() != normals.size(), "Every vertex should have a normal.");

		if (uvs.size() == 0) {
			// TODO Properly generate UVs if there arent any
			uvs = PackedVector2Array();
			uvs.resize(positions.size());
		}

		// Separate triangles belonging to faces of the cube
		BakedData::Surface &surface = baked_data_.surfaces[surface_index];
		Ref<Material> material = materials[surface_index];
		// Note, an empty material counts as "The default material".

		std::array<std::unordered_map<int, int>, SIDE_COUNT> sides_added_indices;
		std::unordered_map<int, int> added_regular_indices;
		std::array<glm::vec3, 3> tri_positions;

		for (int i = 0; i < indices.size(); i += 3) {
			tri_positions[0] = to_vec3(positions[indices[i]]);
			tri_positions[1] = to_vec3(positions[indices[i + 1]]);
			tri_positions[2] = to_vec3(positions[indices[i + 2]]);

			SideAxis side = get_triangle_side(tri_positions[0], tri_positions[1], tri_positions[2], side_vertex_tolerance);
			if (side != SIDE_AXIS_COUNT) {
				// That triangle is on the face

				// 原本的 Mesh 会被分为六个面 + 内部顶点，每个面上的顶点数据都是独立的
				// 因此每个顶点的序号和原 Mesh 的都不一样，需要重新计算
				int new_index = surface.side_positions[side].size();

				for (int j = 0; j < 3; ++j) {
					// 先获取原 Mesh 上的顶点序号
					const int src_index = indices[i + j];
					std::unordered_map<int, int> &added_indices = sides_added_indices[side];
					const auto existing_dst_index_it = added_indices.find(src_index);

					// 判断是否已添加过 src_index
					if (existing_dst_index_it == added_indices.end()) {
						// Add new vertex
						surface.side_indices[side].push_back(new_index);
						surface.side_positions[side].push_back(tri_positions[j]);
						surface.side_uvs[side].push_back(to_vec2(uvs[indices[i + j]]));

						// i is the first vertex of each triangle which increments by steps of 3.
						// There are 4 floats per tangent.
						int ti = indices[i + j] * 4;
						surface.side_tangents[side].push_back(tangents[ti]);
						surface.side_tangents[side].push_back(tangents[ti + 1]);
						surface.side_tangents[side].push_back(tangents[ti + 2]);
						surface.side_tangents[side].push_back(tangents[ti + 3]);

						added_indices.insert({ src_index, new_index });
						++new_index;

					} else {
						// Vertex was already added, just add index referencing it
						// 顶点若已添加，则仅添加索引引用它
						surface.side_indices[side].push_back(existing_dst_index_it->second);
					}
				}

			} else {
				// That triangle is not on the face

				int next_regular_index = surface.positions.size();

				for (int j = 0; j < 3; ++j) {
					const int src_index = indices[i + j];
					const auto existing_dst_index_it = added_regular_indices.find(src_index);

					if (existing_dst_index_it == added_regular_indices.end()) {
						surface.indices.push_back(next_regular_index);
						surface.positions.push_back(tri_positions[j]);
						surface.normals.push_back(to_vec3(normals[indices[i + j]]));
						surface.uvs.push_back(to_vec2f(uvs[indices[i + j]]));

						if (bake_tangents) {
							// i is the first vertex of each triangle which increments by steps of 3.
							// There are 4 floats per tangent.
							int ti = indices[i + j] * 4;
							surface.tangents.push_back(tangents[ti]);
							surface.tangents.push_back(tangents[ti + 1]);
							surface.tangents.push_back(tangents[ti + 2]);
							surface.tangents.push_back(tangents[ti + 3]);
						}

						added_regular_indices.insert({ src_index, next_regular_index });
						++next_regular_index;

					} else {
						surface.indices.push_back(existing_dst_index_it->second);
					}
				}
			}
		}
	}
}

} //namespace pgvoxel
