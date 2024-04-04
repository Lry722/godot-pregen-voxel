#pragma once

#include "constants.h"
#include "voxel_generation_chunk.h"

#include "scene/main/node.h"

namespace pgvoxel::generator {

class VoxelLocalGenerator : public Node {
	GDCLASS(VoxelLocalGenerator, Node);
public:
	GDVIRTUAL1(_generate, Ref<VoxelGenerationChunk>);
private:
	static void _bind_methods();
};

}
