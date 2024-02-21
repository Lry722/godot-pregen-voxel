#pragma once

#include "constants.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/object/object.h"
#include "scene/main/node.h"
#include <cstddef>

namespace pgvoxel {

class VoxelTerrainGenerator : public Node {
	GDCLASS(VoxelTerrainGenerator, Node)
public:
	GDVIRTUAL2(_generate_chunk, size_t, size_t);
	void generate(size_t x, size_t z);

private:
	static void _bind_methods();
	void _notification(int p_what);
};

} //namespace pgvoxel
