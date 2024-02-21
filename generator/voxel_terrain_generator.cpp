#include "voxel_terrain_generator.h"
#include "core/error/error_macros.h"
#include "core/object/object.h"
#include "scene/main/node.h"
#include "voxel_generator.h"

namespace pgvoxel {

void VoxelTerrainGenerator::_bind_methods() {
	GDVIRTUAL_BIND(_generate_chunk, "x", "z");
}

void VoxelTerrainGenerator::generate(size_t x, size_t z) {
	if (!GDVIRTUAL_CALL(_generate_chunk, x, z)) {
		ERR_PRINT("VoxelGenerator::_generate_block is unimplemented!");
	}
}

void VoxelTerrainGenerator::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PARENTED: {
			auto parent = Object::cast_to<VoxelGenerator>(get_parent());
			parent->add_generator(this);
		} break;
		case NOTIFICATION_UNPARENTED: {
			auto parent = Object::cast_to<VoxelGenerator>(get_parent());
			parent->remove_generator(this);
		} break;
	}
}

} //namespace pgvoxel
