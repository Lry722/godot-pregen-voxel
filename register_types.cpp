#include "register_types.h"
#include "core/object/class_db.h"
#include "mesher.h"
#include "voxel_buffer.h"
#include "voxel_generator_layer.h"
#include "voxel_local_generator.h"
#include "voxel_world_config.h"
#include "voxel_world.h"
#include "voxel_block.h"
#include "voxel_generator.h"
#include "voxel_block_library.h"
#include "tests/include/test.h"

void initialize_pgvoxel_module(ModuleInitializationLevel p_level) {
	using namespace pgvoxel;

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<VoxelTest>();
	ClassDB::register_class<VoxelWorldConfig>();
	ClassDB::register_class<VoxelBuffer>();
	ClassDB::register_class<VoxelGenerationChunk>();
	ClassDB::register_class<VoxelGenerator>();
	ClassDB::register_class<VoxelGeneratorLayer>();
	ClassDB::register_class<VoxelLocalGenerator>();
	ClassDB::register_class<VoxelWorld>();
	ClassDB::register_class<VoxelBlock>();
	ClassDB::register_class<VoxelBlockLibrary>();
	ClassDB::register_class<VoxelMesher>();
}

void uninitialize_pgvoxel_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
