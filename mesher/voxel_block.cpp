#include "voxel_block.h"

namespace pgvoxel {

void VoxelBlock::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &VoxelBlock::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &VoxelBlock::get_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");

	ClassDB::bind_method(D_METHOD("get_random_tickable"), &VoxelBlock::get_random_tickable);
	ClassDB::bind_method(D_METHOD("set_random_tickable"), &VoxelBlock::set_random_tickable);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "random_tickable"), "set_random_tickable", "get_random_tickable");

	ADD_GROUP("Culls", "");

	ClassDB::bind_method(D_METHOD("set_culls_neighbors", "culls_neighbors"), &VoxelBlock::set_culls_neighbors);
	ClassDB::bind_method(D_METHOD("get_culls_neighbors"), &VoxelBlock::get_culls_neighbors);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "culls_neighbors"), "set_culls_neighbors", "get_culls_neighbors");

	ClassDB::bind_method(D_METHOD("set_transparency_index", "transparency_index"), &VoxelBlock::set_transparency_index);
	ClassDB::bind_method(D_METHOD("get_transparency_index"), &VoxelBlock::get_transparency_index);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transparency_index"), "set_transparency_index", "get_transparency_index");

	ADD_GROUP("Collision", "");

	ClassDB::bind_method(D_METHOD("set_collision_enabled", "enabled"), &VoxelBlock::set_collision_enabled);
	ClassDB::bind_method(D_METHOD("get_collision_enabled"), &VoxelBlock::get_collision_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_enabled"), "set_collision_enabled", "get_collision_enabled");

	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &VoxelBlock::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &VoxelBlock::get_collision_mask);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");

	ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &VoxelBlock::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &VoxelBlock::get_collision_layer);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_layer", "get_collision_layer");

	ClassDB::bind_method(D_METHOD("set_box_collision_enabled", "enabled"), &VoxelBlock::set_box_collision_enabled);
	ClassDB::bind_method(D_METHOD("get_box_collision_enabled"), &VoxelBlock::get_box_collision_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "box_collision_enabled"), "set_box_collision_enabled", "get_box_collision_enabled");

	ClassDB::bind_method(D_METHOD("set_box_collision_aabbs", "aabbs"), &VoxelBlock::set_box_collision_aabbs);
	ClassDB::bind_method(D_METHOD("get_box_collision_aabbs"), &VoxelBlock::get_box_collision_aabbs);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "box_collision_aabbs", PROPERTY_HINT_TYPE_STRING, String::num_int64(Variant::AABB) + ":"),
			"set_box_collision_aabbs", "get_box_collision_aabbs");
}

} //namespace pgvoxel
