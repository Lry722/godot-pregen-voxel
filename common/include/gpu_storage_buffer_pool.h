#ifndef VOXEL_GPU_STORAGE_BUFFER_H
#define VOXEL_GPU_STORAGE_BUFFER_H

#include "servers/rendering/rendering_device.h"
#include <array>
#include <vector>

namespace pgvoxel {

struct GPUStorageBuffer {
	RID rid;
	size_t size = 0;

	inline bool is_null() const {
		// Can't use `is_null()`, core has it but GDExtension doesn't have it
		return !rid.is_valid();
	}

	inline bool is_valid() const {
		return rid.is_valid();
	}
};

// Pools storage buffers of specific sizes so they can be re-used.
// Not thread-safe.
// 储存一些不同大小的 buffer 以供复用，实际上是 Buffer pool 的 pool，内部的每个 pool 只储存一种大小的 buffer
// 非线程安全
class GPUStorageBufferPool {
public:
	GPUStorageBufferPool();
	~GPUStorageBufferPool();

	void clear();
	void set_rendering_device(RenderingDevice *rd);
	GPUStorageBuffer allocate(const PackedByteArray &pba);
	GPUStorageBuffer allocate(uint32_t p_size);
	void recycle(GPUStorageBuffer b);
	void debug_print() const;

private:
	GPUStorageBuffer allocate(uint32_t p_size, const PackedByteArray *pba);

	unsigned int get_pool_index_from_size(uint32_t p_size) const;

	struct Pool {
		std::vector<GPUStorageBuffer> buffers;
		unsigned int used_buffers = 0;
	};

	static const unsigned int POOL_COUNT = 48;

	std::array<uint32_t, POOL_COUNT> _pool_sizes;
	std::array<Pool, POOL_COUNT> _pools;
	RenderingDevice *_rendering_device = nullptr;
};

} //namespace pgvoxel

#endif // VOXEL_GPU_STORAGE_BUFFER_H
