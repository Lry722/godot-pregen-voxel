#include "register_types.h"
#include "core/object/class_db.h"
#include "voxel_generator_layer.h"
#include "voxel_local_generator.h"
#include "voxel_world_config.h"

#include "voxel_generator.h"
#include "tests/include/test.h"

void initialize_pgvoxel_module(ModuleInitializationLevel p_level) {
	using namespace pgvoxel;

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<VoxelTest>();
	ClassDB::register_class<VoxelWorldConfig>();
	ClassDB::register_class<VoxelGenerationChunk>();
	ClassDB::register_class<VoxelGenerator>();
	ClassDB::register_class<VoxelGeneratorLayer>();
	ClassDB::register_class<VoxelLocalGenerator>();
}

void uninitialize_pgvoxel_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
