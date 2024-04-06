#pragma once

#include "forward.h"
#include "packed_array.h"
#include "palette.h"
#include "modules/pgvoxel/world/include/forward.h"

#include <glm/glm.hpp>
#include <memory>
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
	static const size_t kWidthBits = std::bit_width(kWidth - 1);
	static const size_t kHeightBits = std::bit_width(kHeight - 1);

	// 涉及到坐标转换时都必须按这里规定的zxy的顺序
	static size_t pos_to_index(const Coord pos) {
		return (pos.z << (kWidthBits + kHeightBits)) + (pos.x << (kHeightBits)) + pos.y;
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

	// 序列化
	void serialize(std::ostringstream &oss);
	// 反序列化
	void deserialize(std::istringstream &iss, const size_t size);

private:
	LoadLevels load_levels_;
	const Coord position_;
	Palette palette_;
	PackedArray<> terrain_{ kWidth * kWidth * kHeight };
};

} //namespace pgvoxel
