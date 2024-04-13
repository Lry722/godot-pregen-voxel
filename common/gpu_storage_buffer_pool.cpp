#include "gpu_storage_buffer_pool.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"

#include <algorithm>
#include <sstream>

namespace pgvoxel {

GPUStorageBufferPool::GPUStorageBufferPool() {
	uint32_t s = 1;
	for (unsigned int i = 0; i < _pool_sizes.size(); ++i) {
		// Have sizes aligned to 4 bytes
		// 以 4 字节对齐大小
		_pool_sizes[i] = s * 4;
		// 每次增加 1.5 倍，最大尺寸的 buffer 大约有1个G
		s = std::max(s + 1, s + s / 2);
	}
}

GPUStorageBufferPool::~GPUStorageBufferPool() {
	if (_rendering_device != nullptr) {
		clear();
	}
}

void GPUStorageBufferPool::clear() {
	RenderingDevice &rd = *_rendering_device;
	for (Pool &pool : _pools) {
		for (GPUStorageBuffer &b : pool.buffers) {
			rd.free(b.rid);
		}
		pool.buffers.clear();
	}
}

void GPUStorageBufferPool::set_rendering_device(RenderingDevice *rd) {
	if (_rendering_device != nullptr) {
		// Clear data from the previous device
		// 如果已经设置了渲染设备，则先清除数据
		clear();
	}
	_rendering_device = rd;
}

unsigned int GPUStorageBufferPool::get_pool_index_from_size(uint32_t p_size) const {
	// Find first size that is equal or higher than the given size.
	// (first size that does not satisfy `pool_size < p_size`)
	// 找到第一个尺寸大于 p_size 的 buffer pool 的索引
	auto it = std::lower_bound(_pool_sizes.begin(), _pool_sizes.end(), p_size);
	return it - _pool_sizes.begin();
}

GPUStorageBuffer GPUStorageBufferPool::allocate(const PackedByteArray &pba) {
	return allocate(pba.size(), &pba);
}

GPUStorageBuffer GPUStorageBufferPool::allocate(uint32_t p_size) {
	return allocate(p_size, nullptr);
}

GPUStorageBuffer GPUStorageBufferPool::allocate(uint32_t p_size, const PackedByteArray *pba) {
	RenderingDevice &rd = *_rendering_device;

	const unsigned int pool_index = get_pool_index_from_size(p_size);
	Pool &pool = _pools[pool_index];

	GPUStorageBuffer b;

	if (pool.buffers.size() == 0) {
		const unsigned int capacity = _pool_sizes[pool_index];
		// Unfortunately `storage_buffer_create` in the Godot API requires that you provide a PoolByteArray of the exact
		// same size, when provided. Our pooling strategy means we are often allocating a bit more than initially
		// requested. The passed data would fit, but Godot doesn't want that... so in order to avoid having to create an
		// extended copy of the PackedByteArray, we don't initialize the buffer on creation. Instead, we do it with a
		// separate call... I have no idea if that has a particular performance impact, apart from more RID lookups.
		// b.rid = rd.storage_buffer_create(capacity, pba);
		// 如果要在 storage_buffer_create 函数中初始化 buffer 的内容，需要传入和 buffer 相同大小的数据。
		// 然而我们的 buffer 总是比数据大一些，因此没法在这里进行初始化
		// 因此，只能额外调用一次 buffer_update 函数来传入数据
		b.rid = rd.storage_buffer_create(capacity);
		if (pba != nullptr) {
			rd.buffer_update(b.rid, 0, pba->size(), pba->ptr());
		}
		b.size = capacity;

	} else {
		b = pool.buffers.back();
		pool.buffers.pop_back();
		if (pba != nullptr) {
			rd.buffer_update(b.rid, 0, p_size, pba->ptr());
		}
	}

	++pool.used_buffers;

	return b;
}

void GPUStorageBufferPool::recycle(GPUStorageBuffer b) {
	const unsigned int pool_index = get_pool_index_from_size(b.size);
	Pool &pool = _pools[pool_index];
	--pool.used_buffers;

	pool.buffers.push_back(b);
}

void GPUStorageBufferPool::debug_print() const {
	std::stringstream ss;
	ss << "---- GPUStorageBufferPool ----\n";
	for (unsigned int i = 0; i < _pools.size(); ++i) {
		const Pool &pool = _pools[i];
		if (pool.buffers.capacity() == 0) {
			continue;
		}
		const unsigned int block_size = _pool_sizes[i];
		ss << "Pool[" << i << "] block size: " << block_size << ", pooled buffers: " << pool.buffers.size()
		   << ", capacity: " << pool.buffers.capacity() << "\n";
	}
	ss << "----";
	print_line(ss.str().c_str());
}

} //namespace pgvoxel
