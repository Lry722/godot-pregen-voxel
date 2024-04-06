#include "voxel_world.h"
#include "world_db.h"

namespace pgvoxel {

Dictionary VoxelWorld::getMetadata(int32_t x, int32_t z) {
	return WorldDB::singleton().getMetadata(x, z);
}

void VoxelWorld::setMetadata(int32_t x, int32_t z, const Dictionary &data) {
	WorldDB::singleton().setMetadata(x, z, data);
}

void VoxelWorld::_bind_methods() {
	ClassDB::bind_static_method("VoxelWorld", D_METHOD("get_metadata", "x", "z"), &VoxelWorld::getMetadata);
	ClassDB::bind_static_method("VoxelWorld", D_METHOD("set_metadata", "x", "z", "metadata"), &VoxelWorld::setMetadata);
}

}
