#pragma once

#include "core/object/ref_counted.h"
#include "core/variant/variant.h"
#include "cube.h"

#include "scene/resources/material.h"
#include "scene/resources/mesh.h"

#include <array>
#include <bitset>
#include <cstdint>
#include <glm/glm.hpp>
#include <shared_mutex>
#include <thread>
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

			struct Data {
				std::vector<glm::vec3> positions;
				std::vector<glm::vec3> normals;
				std::vector<glm::vec2> uvs;
				std::vector<int> indices;
				std::vector<float> tangents;
			};


			// 为了在 Meshing 时快速剔除某个面，在预处理时要将内部的数据和面上的数据分离
			Data inside_data;
			Data side_data[Side::SIDE_COUNT];

			void clear() {
				inside_data.positions.clear();
				inside_data.normals.clear();
				inside_data.uvs.clear();
				inside_data.indices.clear();
				inside_data.tangents.clear();

				for (int side = 0; side < SIDE_COUNT; ++side) {
					side_data[side].positions.clear();
					side_data[side].uvs.clear();
					side_data[side].indices.clear();
					side_data[side].tangents.clear();
				}
			}
		};

		std::vector<Surface> surfaces;

		// 用于标记每个面是否有东西，前六位是面，第七位是内部
		uint8_t sides_mask = 0;

		// 对面上的所有三角面进行栅格化结果，记录覆盖到了 pattern 的哪些格子
		// 可通过比较两个面的 pattern 快速判断是否有包含关系，进而判断是否需要剔除
		// 16 x 16 的精度应该足够了
		static const uint8_t PATTERN_WIDTH = 16;
		std::array<std::bitset<PATTERN_WIDTH * PATTERN_WIDTH>, Side::SIDE_COUNT> side_pattern;

		void clear() {
			for (unsigned int i = 0; i < surfaces.size(); ++i) {
				surfaces[i].clear();
			}
			sides_mask = 0;
		}
	};

	// 预处理
	void bake();
	bool baked() const {return baked_;}
	const BakedData &get_baked_data() const {
		return baked_data_;
	}

	void set_mesh(Ref<Mesh> mesh) {
		if (mesh == mesh_) {
			return;
		}
		mesh_ = mesh;
		baked_ = false;
		baked_data_.clear();
	}
	Ref<Mesh> get_mesh() const { return mesh_; }

	const float side_vertex_tolerance{ 0.001 };

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
	bool baked_{ false };

	// 原始 Mesh 数据
	Ref<Mesh> mesh_;
	// 预处理后的数据
	BakedData baked_data_;
};

} //namespace pgvoxel
