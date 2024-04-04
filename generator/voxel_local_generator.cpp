#include "voxel_local_generator.h"
#include "core/object/object.h"

namespace pgvoxel::generator {

void VoxelLocalGenerator::_bind_methods() {
	GDVIRTUAL_BIND(_generate, "chunk")
}

}
