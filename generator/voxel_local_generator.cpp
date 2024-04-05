#include "voxel_local_generator.h"

namespace pgvoxel{

void VoxelLocalGenerator::_bind_methods() {
	GDVIRTUAL_BIND(_generate, "chunk")
}

}
