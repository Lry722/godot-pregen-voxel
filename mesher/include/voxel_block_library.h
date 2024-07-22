#pragma once

#include "core/error/error_macros.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "voxel_block.h"

#include "core/variant/typed_array.h"

#include "modules/pgvoxel/thirdparty/thread-pool/include/BS_thread_pool.hpp"

#include <atomic>
#include <cstddef>
#include <vector>

namespace pgvoxel {

class VoxelBlockLibrary : public Resource {
	GDCLASS(VoxelBlockLibrary, Resource)
public:
	int get_block_id(const String &block_name) const {
		for (size_t i = 0; i < library_.size(); ++i) {
			if (static_cast<Ref<VoxelBlock>>(library_[i])->get_name() == block_name) {
				return i;
			}
		}
	}

	bool has_block(size_t index) const {
		return index < library_.size();
	}

	Ref<VoxelBlock> get_block(size_t index) const {
		return library_[index];
	}

	void set_block(size_t index, Ref<VoxelBlock> block) {
		library_[index] = block;
	}

	void set_library(TypedArray<VoxelBlock> library) {
		library_.resize(library.size());
		for (int i = 0; i < library.size(); ++i) {
			library_[i] = library[i];
		}
	}
	TypedArray<VoxelBlock> get_library() const {
		TypedArray<VoxelBlock> array;
		array.resize(library_.size());
		for (size_t i = 0; i < library_.size(); ++i) {
			array[i] = library_[i];
		}
		return array;
	}

	void bake() {
		static BS::thread_pool bake_thread_pool_;
		static bool is_baking{ false };

		ERR_FAIL_COND(is_baking);
		is_baking = true;
		for (size_t i = 0; i < library_.size(); ++i) {
			if (!library_[i]->baked()) {
				(void)bake_thread_pool_.submit_task([&]() {
					library_[i]->bake();
				});
			}
		}
		bake_thread_pool_.wait();
		is_baking = false;
	}

private:
	static void _bind_methods();

	std::vector<Ref<VoxelBlock>> library_;
};

} //namespace pgvoxel
