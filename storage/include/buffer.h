#pragma once

#include "core/error/error_macros.h"
#include "forward.h"
#include <cstdint>
#include <glm/fwd.hpp>
#include <sstream>
#include <vector>

namespace pgvoxel {

// 和Chunk不同，Buffer通常用于储存需要频繁读取的小数据，因此不采用Palette压缩
class Buffer {
public:
	uint64_t pos_to_index(const Coord &pos) const {
		return pgvoxel::pos_to_index(pos, width_bits, height_bits);
	}

public:
	Buffer(CoordAxis width, CoordAxis height, CoordAxis depth) :
			width_(width), height_(height), depth_(depth), width_bits(std::bit_width(width - 1)), height_bits(std::bit_width(height - 1)), data_(width * height * depth) {}

	void init(const CoordAxis width, const CoordAxis height, const CoordAxis depth) {
		ERR_FAIL_COND_MSG(width_ != 0 || height_ != 0 || depth_ != 0, "Buffer can only be initialized once time.");
		width_ = width;
		height_ = height;
		depth_ = depth;
		width_bits = std::bit_width(width - 1);
		height_bits = std::bit_width(height - 1);
		data_.resize(width * height * depth);
	}

	CoordAxis getWidth() const { return width_; }
	CoordAxis getHeight() const { return height_; }
	CoordAxis getDepth() const { return depth_; }

	// 单点操作
	void setVoxel(const Coord pos, const VoxelData data);
	VoxelData getVoxel(const Coord pos) const;

	// 竖列操作
	// 只有垂直方向上的数据是连续的，因此 setBar 只能处理竖列
	void setBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top, const VoxelData data);
	void setBar(const Coord &pos, const std::vector<VoxelData> &data);
	std::vector<VoxelData> getBar(const CoordAxis x, const CoordAxis z, const CoordAxis buttom, const CoordAxis top) const;

	// 块操作，等效于遍历块的底面，依次调用竖列操作
	// 超出的部分会被忽略
	void setBlock(const Coord begin, const Coord end, const VoxelData data);
	void setBlock(const Coord pos, const Buffer &data);
	Buffer getBlock(const Coord begin, const Coord end) const;

	// 序列化/反序列化
	void serialize(std::ostringstream &oss) const;
	void deserialize(std::istringstream &iss, const uint32_t size);

private:
	CoordAxis width_, height_, depth_;
	uint8_t width_bits, height_bits;

	// 在生成时一个buffer（比如一棵树）可能会被访问数百万遍，为了快速读取，加之本身体积不大，故不采用压缩
	std::vector<VoxelData> data_;
};

} //namespace pgvoxel
