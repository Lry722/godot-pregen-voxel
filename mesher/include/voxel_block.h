#pragma once

#include "core/error/error_macros.h"
#include "core/io/resource.h"
#include "core/object/object.h"
#include "model.h"

namespace pgvoxel {

class VoxelMesher;

// Block表示地形中一个基础的格子的内容和属性
class VoxelBlock : public Resource {
	GDCLASS(VoxelBlock, Resource)
public:
	friend class pgvoxel::VoxelMesher;

public:
	// Model 相关属性
	void set_mesh(Ref<Mesh> mesh) { model.set_mesh(mesh); }
	Ref<Mesh> get_mesh() const { return model.get_mesh(); }
	void bake() {
		set_current_thread_safe_for_nodes(true);
		print_verbose("Prepare baking");
		if (model.get_mesh().is_valid()) {
			print_verbose(String("Start baking."));
			print_verbose(String("Mesh rid {0}").format(varray(model.get_mesh()->get_rid())));
			model.bake();
		} else {
			print_verbose("No mesh to bake.");
		}
	}
	bool baked() const { return model.baked(); }
	const Model &get_model() const { return model; }

	// 随机tick相关属性
	void set_random_tickable(bool tickable) { random_tickable = tickable; }
	bool get_random_tickable() const { return random_tickable; }

	// 邻接方块剔除相关属性
	void set_culls_neighbors(bool enable_culling) { model.culls_neighbors = enable_culling; }
	bool get_culls_neighbors() const { return model.culls_neighbors; }
	void set_transparency_index(uint8_t index) { model.transparency_index = index; }
	uint8_t get_transparency_index() const { return model.transparency_index; }

	// 碰撞相关属性
	void set_collision_enabled(bool enable) { model.collision_enabled = enable; }
	bool get_collision_enabled() const { return model.collision_enabled; }
	void set_collision_layer(uint32_t layer) { model.collision_layer = layer; }
	uint32_t get_collision_layer() const { return model.collision_layer; }
	void set_collision_mask(uint32_t mask) { model.collision_mask = mask; }
	uint32_t get_collision_mask() const { return model.collision_mask; }

	// AABB盒碰撞体积相关属性
	void set_box_collision_enabled(bool enable) { model.box_collision_enabled = enable; }
	bool get_box_collision_enabled() const { return model.box_collision_enabled; }
	void set_box_collision_aabbs(const TypedArray<AABB> &aabbs) {
		model.box_collision_aabbs.clear();
		model.box_collision_aabbs.resize(aabbs.size());
		for (int i = 0; i < aabbs.size(); ++i) {
			const Variant &aabb = aabbs[i];
			ERR_CONTINUE_MSG(aabb.get_type() != Variant::AABB, vformat("Item {} of the array is not an AABB (found {}).", i));
			model.box_collision_aabbs[i] = aabb;
		}
		emit_changed();
	}
	TypedArray<AABB> get_box_collision_aabbs() const {
		TypedArray<AABB> aabbs;
		aabbs.resize(model.box_collision_aabbs.size());
		for (size_t i = 0; i < model.box_collision_aabbs.size(); ++i) {
			aabbs[i] = model.box_collision_aabbs[i];
		}
		return aabbs;
	}

private:
	static void _bind_methods();

	Model model;

	// 该 model 是否会成为 random tick 的目标
	bool random_tickable = false;
};

} //namespace pgvoxel
