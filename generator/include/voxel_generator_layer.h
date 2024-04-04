#pragma once

#include "constants.h"

#include "core/object/gdvirtual.gen.inc"
#include "core/object/object.h"
#include "scene/main/node.h"
#include "voxel_chunk.h"
#include "voxel_generation_chunk.h"
#include <cstddef>
#include <vector>

namespace pgvoxel::generator {

class VoxelGeneratorLayer : public Node {
	GDCLASS(VoxelGeneratorLayer, Node)
public:
	void generate(std::vector<Ref<VoxelGenerationChunk>> &chunks);
	PackedStringArray get_configuration_warnings() const override;
private:
	static void _bind_methods();
};

} //namespace pgvoxel
