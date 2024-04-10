#pragma once

#include "serialize.h"
#include "chunk.h"
#include "forward.h"
#include "palette.inl"

#include <lz4.h>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace pgvoxel {

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setVoxel(const Coord pos, const VoxelData new_data) {
	auto old_data_index = terrain_[pos_to_index(pos)];

	const auto new_data_index = palette_.update(new_data, old_data_index);
	if (palette_.paletteSize() > terrain_.elementCapacity()) {
		terrain_.grow();
	}

	old_data_index = new_data_index;
}

template <CoordAxis kWidth, CoordAxis kHeight>
VoxelData Chunk<kWidth, kHeight>::getVoxel(const Coord pos) const {
	return palette_.pick(terrain_[pos_to_index(pos)]);
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data) {
	const auto begin = pos_to_index({ x, buttom, z });
	const auto end = pos_to_index({ x, top, z });
	auto covered = terrain_.getRange(begin, end);

	for (size_t i = 0; i < covered.size(); ++i) {
		palette_.update(data, covered[i]);
	}
	// 由于最多只会添加一个元素，因此最多只需要增长一次
	if (palette_.paletteSize() > terrain_.elementCapacity()) {
		terrain_.grow();
	}

	terrain_.setRange(begin, end, palette_.indexOf(data));
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const std::vector<VoxelData> &data) {
	// 顺序必须按vec3_to_index中规定的zxy，否则top作为尾后迭代器，会覆盖到不属于自己的位
	// 最低位为y轴，而y轴为垂直方向，因此terrain在垂直方向上的数据是连续的，这就是为什么只能set竖直的bar
	const auto begin = pos_to_index({ x, buttom, z });
	const auto end = pos_to_index({ x, top, z });
	auto covered = terrain_.getRange(begin, end);

	std::vector<VoxelData> result;
	result.reserve(covered.size());
	VoxelData max_data_index{ 0 };

	for (size_t i = 0; i < covered.size(); ++i) {
		result.push_back(palette_.update(data[i], covered[i]));
		max_data_index = std::max(max_data_index, result[i]);
	}
	
	// 可能会添加多个新元素，需要不断增长到可容纳所有新元素
	while (palette_.paletteSize() > terrain_.elementCapacity()) {
		terrain_.grow();
	}

	terrain_.setRange(begin, end, result);
}

template <CoordAxis kWidth, CoordAxis kHeight>
std::vector<VoxelData> Chunk<kWidth, kHeight>::getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top) const {
	const auto &dataIndeies = terrain_.getRange(pos_to_index({ x, buttom, z }), pos_to_index({ x, top, z }));
	std::vector<VoxelData> result;
	result.resize(dataIndeies.size());
	for (size_t i = 0; i < dataIndeies.size(); ++i) {
		result[i] = palette_.pick(dataIndeies[i]);
	}
	return result;
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setBlock(const Coord begin, const Coord end, const VoxelData data) {
	for (CoordAxis x = begin.x; x < end.x; ++x) {
		for (CoordAxis z = begin.z; z < end.z; ++z) {
			setBar(x, z, begin.y, end.y, data);
		}
	}
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::setBlock(const Coord position, const Buffer &data) {
	for (CoordAxis dx = 0; dx < data.kWidth && position.x + dx < kWidth; ++dx) {
		for (CoordAxis dz = position.z; dz < data.kWidth && position.z + dz < kWidth; ++dz) {
			setBar(position.x + dx, position.z + dz, position.y, position.y + data.kHeight,
					data.getBar(dx, dz, 0, data.kHeight));
		}
	}
}

template <CoordAxis kWidth, CoordAxis kHeight>
Buffer Chunk<kWidth, kHeight>::getBlock(const Coord begin, const Coord end) const {
	Buffer result(end.x - begin.x, end.y - begin.y, end.z - begin.z);
	for (CoordAxis dx = 0; dx < result.kWidth; ++dx) {
		for (CoordAxis dz = 0; dz < result.kWidth; ++dz) {
			result.setBar(dx, dz, 0, result.kHeight,
					getBar(begin.x + dx, begin.z + dz, begin.y, begin.y + result.kHeight));
		}
	}
	return result;
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::serialize(std::ostringstream &oss) const {
	// 生成原始数据
	std::ostringstream oss_uncompressed;
	oss_uncompressed << palette_;
	oss_uncompressed << terrain_;
	auto data = oss_uncompressed.view();

	// 写入原始数据的大小，此处size表示原始数据的大小
	std::size_t size{ data.size() };
	oss.write(reinterpret_cast<const char *>(&size), sizeof(size));

	// 使用LZ4压缩
	std::string buffer;
	buffer.resize(LZ4_COMPRESSBOUND(size));
	// 此处size表示压缩后数据的大小
	size = LZ4_compress_default(data.data(), buffer.data(), size, buffer.size());
	if (size <= 0) [[unlikely]] {
		throw std::runtime_error(std::format("Chunk ({}, {}, {}): Compression failed!", position_.x, position_.y, position_.z));
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

	// 读取压缩数据
	std::string compressedData;
	compressedData.resize(size);
	iss.read(compressedData.data(), size);

	// 使用LZ4解压
	int decompressedSize = LZ4_decompress_safe(compressedData.data(), decompressedData.data(), size - sizeof(original_size), original_size);
	if (decompressedSize <= 0) [[unlikely]] {
		throw std::runtime_error(std::format("Chunk ({}, {}, {}): Decompression failed!", position_.x, position_.y, position_.z));
	}

	// 将解压后的数据转换为原始数据格式
	std::istringstream iss_uncompressed(decompressedData);
	iss_uncompressed >> palette_ >> terrain_;
}

template <CoordAxis kWidth, CoordAxis kHeight>
std::string Chunk<kWidth, kHeight>::toString() const {
	std::ostringstream oss;
	oss << "Chunk {\n"
		<< "position: (" << position_.x << ", " << position_.y << ", " << position_.z << ")\n"
		<< "palette:\n"
		<< palette_.toString() << "\n"
		<< "terrain:\n"
		<< terrain_.toString() << "\n"
		<< "}";
	return oss.str();
}

template <CoordAxis kWidth, CoordAxis kHeight>
void Chunk<kWidth, kHeight>::fit() {
	palette_.fit();
	terrain_.fit();
}

} // namespace pgvoxel
