#pragma once

#include "constants.h"
#include "chunk.h"

#include "core/object/ref_counted.h"
#include "core/object/object.h"
#include "core/string/print_string.h"
#include <cstddef>
#include <memory>

namespace pgvoxel::generator {

class VoxelGenerationChunk : public RefCounted {
	GDCLASS(VoxelGenerationChunk, RefCounted)
public:
	VoxelGenerationChunk() = default;
	VoxelGenerationChunk(size_t x, size_t z);
private:
	static void _bind_methods();

	bool initialized{false};
	std::unique_ptr<GenerationChunk> data;
};

} //namespace pgvoxel::generator
