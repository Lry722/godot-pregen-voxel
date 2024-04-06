// clangd-add-tu chunk_inl.cpp
#pragma once

#include "chunk.h"
#include "core/string/print_string.h"
#include "forward.h"
#include "packed_array.inl"
#include "serialize.h"

#include <lz4.h>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace pgvoxel{

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setVoxel(const Coord pos, const VoxelData new_data) {
	auto old_data = terrain_[pos_to_index(pos)];
	if (old_data == new_data) {
		return;
	}

	if (palette_.update(new_data, old_data) && palette_.size() > terrain_.elementCapacity()) {
		terrain_.grow();
	}

	old_data = new_data;
}

template <CoordAxis kWidth, CoordAxis kHeight>
VoxelData Chunk<kWidth, kHeight>::getVoxel(const Coord pos) const {
	return palette_.pick(terrain_[pos_to_index(pos)]);
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data) {
	// 顺序必须按vec3_to_index中规定的zxy，否则top作为尾后迭代器，会覆盖到不属于自己的部分
	// 最低位为y轴，而y轴为垂直方向，因此terrain在垂直方向上的数据是连续的，这就是为什么只能set竖直的bar
	terrain_.setRange(pos_to_index({x, buttom, z}), pos_to_index({x, top, z}), data);
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const std::vector<VoxelData> &data) {
	// 顺序必须按vec3_to_index中规定的zxy，否则top作为尾后迭代器，会覆盖到不属于自己的部分
	// 最低位为y轴，而y轴为垂直方向，因此terrain在垂直方向上的数据是连续的，这就是为什么只能set竖直的bar
	terrain_.setRange(pos_to_index({x, buttom, z}), pos_to_index({x, top, z}), data);
}

template <CoordAxis kWidth, CoordAxis kHeight>
std::vector<VoxelData> Chunk<kWidth, kHeight>::getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top) const {
	// 和setBar同理
	try{
	return terrain_.getRange(pos_to_index({x, buttom, z}), pos_to_index({x, top, z}));}
	catch (const std::length_error &e) {
		throw std::length_error(std::format("Chunk::getBar: begin ({}, {}, {}) end ({}, {}, {}) beginIndex {} endIndex{}", 
			x, buttom, z, x, top, z, pos_to_index({x, buttom, z}), pos_to_index({x, top, z})));
	}
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setBlock(const Coord begin, const Coord end, const VoxelData data) {
	for (auto x = begin.x; x < end.x; ++x) {
		for (auto z = begin.z; z < end.z; ++z) {
			setBar(x, z, begin.y, end.y, data);
		}
	}
}

template <CoordAxis kWidth, CoordAxis kHeight>
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
		print_error(String("Chunk({0}): Compression failed!").format(varray(toVector3i(position_))));
	}

	oss.write(buffer.data(), size);
}

template <CoordAxis kWidth, CoordAxis kHeight>
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
		print_error(String("Chunk({0}): Decompression failed!").format(varray(toVector3i(position_))));
	}

	// 将解压后的数据转换为原始数据格式
	std::istringstream iss_uncompressed(decompressedData);
	iss_uncompressed >> palette_ >> terrain_;
}
} // namespace pgvoxel
