#pragma once

#include "core/object/ref_counted.h"
#include "cube.h"

#include "scene/resources/material.h"
#include "scene/resources/mesh.h"

#include <array>
#include <bitset>
#include <glm/glm.hpp>
#include <vector>

namespace pgvoxel {

// 主要用于预处理 Mesh，方便在 Meshing 时快速剔除与相邻方块紧贴的面
// 包含生成地形 Mesh 和 碰撞 Mesh 所需的所有信息
struct Model {
	struct BakedData {
		// 这里的 Surface 指的是一个材质对应的所有三角面，而不是单个三角面
		struct Surface {
			// 该面的材质
			Ref<Material> material;

			// Inside part of the model.
			// 内部的三角面数据
			std::vector<glm::vec3> positions;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> uvs;
			std::vector<int> indices;
			std::vector<float> tangents;
			// Model sides:
			// They are separated because this way we can occlude them easily.
			// Due to these defining cube side triangles, normals are known already.
			// 六个面上的三角面数据
			// 为了在 Meshing 时快速剔除某个面，在预处理时将它们分开储存
			// 因为六个面的方向固定，因此法线已知，不用储存
			std::array<std::vector<glm::vec3>, SIDE_COUNT> side_positions;
			std::array<std::vector<glm::vec2>, SIDE_COUNT> side_uvs;
			std::array<std::vector<int>, SIDE_COUNT> side_indices;
			std::array<std::vector<float>, SIDE_COUNT> side_tangents;

			void clear() {
				positions.clear();
				normals.clear();
				uvs.clear();
				indices.clear();
				tangents.clear();

				for (int side = 0; side < SIDE_COUNT; ++side) {
					side_positions[side].clear();
					side_uvs[side].clear();
					side_indices[side].clear();
					side_tangents[side].clear();
				}
			}
		};

		std::vector<Surface> surfaces;

		// 用于标记每个面是否有东西，前六位是面，第七位是内部
		uint8_t sides_mask = 0;

		// 每个面上的所有三角面的 8*8 的栅格化结果
		// 可通过比较两个面的 pattern 判断是否有包含关系，进一步判断是否需要剔除被包含的相邻的面
		std::array<std::bitset<64>, Side::SIDE_COUNT> side_pattern_indices;

		void clear() {
			for (unsigned int i = 0; i < surfaces.size(); ++i) {
				surfaces[i].clear();
			}
			sides_mask = 0;
		}
	};

	// 预处理
	void bake();
	const BakedData &get_baked_data() const {
		return baked_data_;
	}

	void set_mesh(Ref<Mesh> mesh) {
		if (mesh == mesh_) {
			return;
		}
		mesh_ = mesh;
		baked = false;
	}
	Ref<Mesh> get_mesh() const { return mesh_; }

	const float side_vertex_tolerance{0.001};

	// 是否启用剔除相邻方块
	bool culls_neighbors{ true };
	// 如果启用了剔除，相邻方块的 transparency index 相同时，会发生剔除
	uint8_t transparency_index{ 0 };

	// 是否启用碰撞
	bool collision_enabled{ true };
	// 所处的碰撞层
	uint32_t collision_layer{ 0 };
	// 所扫描的碰撞层
	uint32_t collision_mask{ 0 };
	// 是否用 AABB 盒替代 Mesh 生成碰撞体积
	bool box_collision_enabled = true;
	// 如果启用了 box collision，则会使用这里储存的 AABB 盒
	std::vector<AABB> box_collision_aabbs{};

private:
	bool baked{ false };

	// 原始 Mesh 数据
	Ref<Mesh> mesh_;
	// 预处理后的数据
	BakedData baked_data_;
};

} //namespace pgvoxel
