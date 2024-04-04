#pragma once

#include "constants.h"
#include "packed_array.h"
#include "palette.h"
#include "viewer.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

namespace pgvoxel::storage {

template <size_t kWidth, size_t kHeight>
class Chunk {
public:
	friend class WorldDB;
	static std::unique_ptr<Chunk<kWidth, kHeight>> create(const size_t x, const size_t z) {
		return std::make_unique<Chunk<kWidth, kHeight>>(x, z);
	}

public:
	Chunk(const size_t x, const size_t z) :
			x_(x), z_(z) {}
	// 应该使用智能指针来管理Chunk，不应该出现拷贝
	Chunk(const Chunk<kWidth, kHeight> &other) = delete;
	Chunk<kWidth, kHeight> &operator=(const Chunk<kWidth, kHeight> &other) = delete;

	// 设置一个点的值
	void setVoxel(const Vec3 pos, const VoxelData data);
	// 获取一个点的值
	VoxelData getVoxel(const Vec3 pos) const;
	// 设置位于(x, z)处，从buttom到top间的长条的值，在修改大量值时效率高于逐个调用setVoxel
	void setBar(const size_t x, const size_t z, const size_t buttom, const size_t top, const VoxelData data);
	// 设置begin到end两点围成的区域中的值，效果等同于遍历水平面，逐个调用setBar
	void setBlock(const Vec3 begin, const Vec3 end, const VoxelData data);

	// 序列化
	void serialize(std::ostringstream &oss);
	// 反序列化
	void deserialize(std::istringstream &iss, const size_t size);

private:

	world::LoadLevels load_levels_;
	const int32_t x_, z_;
	Palette palette_;
	PackedArray<> terrain_{ kWidth * kWidth * kHeight };
};

} //namespace pgvoxel
