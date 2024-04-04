#pragma once
#include "chunk.h"
#include "constants.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include <cstddef>

namespace pgvoxel::storage {

class VoxelChunk : RefCounted {
	GDCLASS(VoxelChunk, RefCounted)
public:
	VoxelChunk(size_t x, size_t z);
private:
	static void _bind_method();

	storage::LoadedChunk data;
};

} //namespace pgvoxel
