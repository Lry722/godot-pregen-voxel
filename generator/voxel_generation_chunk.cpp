#include "voxel_generation_chunk.h"
#include "constants.h"
#include <cstddef>
#include <memory>

namespace pgvoxel::generator {

VoxelGenerationChunk::VoxelGenerationChunk(size_t x, size_t z) {
	data = GenerationChunk::create(x, z);
	initialized = true;
}

void VoxelGenerationChunk::_bind_methods() {

}

}
