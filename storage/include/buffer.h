#pragma once

#include "forward.h"
#include "packed_array.h"
#include <cstdint>
#include <glm/fwd.hpp>
#include <vector>

namespace pgvoxel {

// 用来存地物和结构等数据
// 和Chunk不同，Buffer通常用于储存小数据，且需要快速读取，因此不采用Palette压缩
class Buffer {
public:
	const CoordAxis kWidth, kHeight, kDepth;
public:
	Buffer(size_t width, size_t height, size_t depth) :
			kWidth(width), kHeight(height), kDepth(depth), kWidthBits(std::bit_width(width - 1)), kHeightBits(std::bit_width(height - 1)), data_(width * height * depth) {}

	// 设置一个点的值
	void setVoxel(const Coord pos, const VoxelData data);
	// 获取一个点的值
	VoxelData getVoxel(const Coord pos) const;
	// 设置位于(x, z)处，从buttom到top间的数据为data
	void setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data);
	// 设置位于(x, z)处，从buttom到top间的数据为data，data的size应当大于等于 top - buttom
	void setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const std::vector<VoxelData> &data);
	// 获取位于(x, z)处，从buttom到top间的数据
	std::vector<VoxelData> getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top) const;
	// 设置begin到end两点围成的区域中的值，效果等同于遍历水平面，逐个调用setBar
	void setBlock(const Coord begin, const Coord end, const VoxelData data);

	// 序列化
	void serialize(std::ostringstream &oss);
	// 反序列化
	void deserialize(std::istringstream &iss, const size_t size);

private:
	const uint8_t kWidthBits, kHeightBits;
	size_t pos_to_index(const Coord &pos) const {
		return pgvoxel::pos_to_index(pos, kWidthBits, kHeightBits);
	}

	// 在生成时一个buffer（比如一棵树）可能会被访问数百万遍，为了快速读取，加之本身体积不大，故不采用压缩
	std::vector<VoxelData> data_;
};

} //namespace pgvoxel
