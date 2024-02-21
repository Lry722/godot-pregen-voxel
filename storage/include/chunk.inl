// clangd-add-tu chunk_inl.cpp
#pragma once

#include "chunk.h"
#include "packed_array.inl"
#include "serialize.h"

#include <lz4.h>
#include <sstream>
#include <stdexcept>

namespace pgvoxel {
static inline size_t vec3_to_index(const Vec3 position) {
	return position.z << 8 | position.x << 4 | position.y;
}

template <size_t kWidth, size_t kHeight>
void Chunk<kWidth, kHeight>::setVoxel(const Vec3 pos, const VoxelData new_data) {
	auto old_data = terrain_[vec3_to_index(pos)];
	if (old_data == new_data) {
		return;
	}

	if (palette_.update(new_data, old_data) && palette_.size() > terrain_.elementCapacity()) {
		terrain_.grow();
	}

	old_data = new_data;
}

template <size_t kWidth, size_t kHeight>
VoxelData Chunk<kWidth, kHeight>::getVoxel(const Vec3 pos) const {
	return palette_.pick(terrain_[vec3_to_index(pos)]);
}

template <size_t kWidth, size_t kHeight>
void Chunk<kWidth, kHeight>::serialize(std::ostringstream &oss) {
	// 生成原始数据
	std::ostringstream oss_uncompressed;
	oss_uncompressed << palette_ << terrain_;
	auto data = oss_uncompressed.view();

	// 写入原始数据的大小，此处size表示原始数据的大小
	std::size_t size{ data.size() };
	oss.write(reinterpret_cast<const char *>(&size), sizeof(size));

	// 使用LZ4压缩
	std::string buffer;
	buffer.resize(LZ4_COMPRESSBOUND(size));
	// 此处size表示压缩后数据的大小
	size = LZ4_compress_default(data.data(), buffer.data(), data.size(), buffer.size());
	if (size <= 0) [[unlikely]] {
		throw std::runtime_error(std::format("Chunk({}, {}): Compression failed!", x_, z_));
	}

	oss.write(buffer.data(), size);
}

template <size_t kWidth, size_t kHeight>
void Chunk<kWidth, kHeight>::deserialize(std::istringstream &iss, const size_t size) {
	// 读取原始数据的大小
	std::size_t original_size;
	iss.read(reinterpret_cast<char *>(&original_size), sizeof(original_size));

	// 分配足够的内存来存储解压后的数据
	std::string decompressedData;
	decompressedData.resize(original_size);

	// 分配内存来存储压缩数据
	const std::size_t target_size = size - sizeof(original_size);
	std::string compressedData;
	compressedData.resize(size);

	// 读取压缩数据
	iss.read(compressedData.data(), size);

	// 使用LZ4解压
	int decompressedSize = LZ4_decompress_safe(compressedData.data(), decompressedData.data(), target_size, original_size);
	if (decompressedSize <= 0) [[unlikely]] {
		throw std::runtime_error("Decompression failed");
	}

	// 将解压后的数据转换为原始数据格式
	std::istringstream iss_uncompressed(decompressedData);
	iss_uncompressed >> palette_ >> terrain_;
}
} // namespace pgvoxel
