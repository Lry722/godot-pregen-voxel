#include "core/object/class_db.h"
#include "core/object/object.h"
#include "voxel_block_library.h"

namespace pgvoxel {

void VoxelBlockLibrary::_bind_methods() {
	ClassDB::bind_method(D_METHOD("bake"), &VoxelBlockLibrary::bake);

	ClassDB::bind_method(D_METHOD("set_block", "block"), &VoxelBlockLibrary::set_block);
	ClassDB::bind_method(D_METHOD("get_block"), &VoxelBlockLibrary::get_block);

	ClassDB::bind_method(D_METHOD("set_library", "library"), &VoxelBlockLibrary::set_library);
	ClassDB::bind_method(D_METHOD("get_library"), &VoxelBlockLibrary::get_library);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "content", PROPERTY_HINT_ARRAY_TYPE, MAKE_RESOURCE_TYPE_HINT(VoxelBlock::get_class_static())),
			"set_library", "get_library");
}

} //namespace pgvoxel
