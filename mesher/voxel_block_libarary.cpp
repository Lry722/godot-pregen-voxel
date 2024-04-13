#include "core/object/class_db.h"
#include "core/object/object.h"
#include "voxel_block_library.h"

namespace pgvoxel {

void VoxelBlockLibrary::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_library", "library"), &VoxelBlockLibrary::set_library);
	ClassDB::bind_method(D_METHOD("get_library"), &VoxelBlockLibrary::get_library);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "content"), "set_library", "get_library");
}

}
