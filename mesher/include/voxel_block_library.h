#pragma once

#include "core/variant/typed_array.h"
#include "voxel_block.h"
#include <cstddef>

namespace pgvoxel {

class VoxelBlockLibrary : public Resource {
	GDCLASS(VoxelBlockLibrary, Resource)
public:

	size_t get_block_id(const String &block_name) const {
		for (size_t i = 0; i < library_.size(); ++i) {
		    if (static_cast<Ref<VoxelBlock>>(library_[i])->get_name() == block_name) {
				return i;
		    }
		}
	}

	const Ref<VoxelBlock> get_block(size_t index) const {
	    return library_[index];
	}

	void set_library(const TypedArray<Ref<VoxelBlock>> &library) {
		library_ = library;
	}
	const TypedArray<Ref<VoxelBlock>> &get_library() const {
	    return library_;
	}

private:
	static void _bind_methods();

	TypedArray<Ref<VoxelBlock>> library_;
};

} //namespace pgvoxel
