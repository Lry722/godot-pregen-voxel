#pragma once

#include "buffer.h"
#include "forward.h"
#include "packed_array.h"
#include "palette.h"
#include "../../world/include/forward.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace pgvoxel {


// 区块是一个长宽均为kWidth，高为kHeight的立方体
// 局部坐标按照vec3_to_index的规定映射到data中的序号
template <CoordAxis kWidth, CoordAxis kHeight>
class Chunk {
public:
	friend class WorldDB;

	static std::unique_ptr<Chunk<kWidth, kHeight>> create(const Coord &position) {
		return std::make_unique<Chunk<kWidth, kHeight>>(position);
	}

	// 2 * kWidthBits + kHeightBits 应小于 size_t的位数
	inline static const size_t kWidthBits = std::bit_width(kWidth - 1);
	inline static const size_t kHeightBits = std::bit_width(kHeight - 1);
	static size_t pos_to_index(const Coord &pos) {
		return pgvoxel::pos_to_index(pos,kWidthBits, kHeightBits);
	}

public:
	Chunk(const Coord &position) :
			position_(position) {}
	// 应该使用智能指针来管理Chunk，不应该出现拷贝
	Chunk(const Chunk<kWidth, kHeight> &other) = delete;
	Chunk<kWidth, kHeight> &operator=(const Chunk<kWidth, kHeight> &other) = delete;

	Coord position() const { return position_; }

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
	// 设置position处的等同于data尺寸的block中值，超出当前区块的部分会被忽略
	void setBlock(const Coord position, const Buffer &data);
	// 获取begin到end间的值，以Buffer的形式储存
	Buffer getBlock(const Coord begin, const Coord end) const;

	// 序列化
	void serialize(std::ostringstream &oss) const;
	// 反序列化
	void deserialize(std::istringstream &iss, const size_t size);
	std::string toString() const;

	// 尝试清除冗余数据
	void fit();

private:
	LoadLevels load_levels_;
	const Coord position_;
	Palette<VoxelData, VoxelData> palette_;
	PackedArray terrain_{ kWidth * kWidth * kHeight };
};

} //namespace pgvoxel
