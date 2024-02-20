#pragma once

#include "constants.h"
#include "packed_array.h"
#include "palette.h"
#include "viewer.h"

#include <cstdint>
#include <glm/glm.hpp>

namespace pgvoxel {

template <size_t kWidth, size_t kHeight>
class Chunk {
public:
	friend class WorldDB;
	static std::unique_ptr<Chunk<kWidth, kHeight>> create(const size_t x, const size_t z) {
		return std::unique_ptr<Chunk<kWidth, kHeight>>(new Chunk<kWidth, kHeight>(x, z));
	}

public:
	// 应该使用智能指针来管理Chunk，不应该出现拷贝
	Chunk(const Chunk<kWidth, kHeight> &other) = delete;
	Chunk<kWidth, kHeight> &operator=(const Chunk<kWidth, kHeight> &other) = delete;

	void setVoxel(const Vec3 pos, const VoxelData data);
	VoxelData getVoxel(const Vec3 pos) const;
	// 序列化
	void serialize(std::ostringstream &oss);
	// 反序列化
	void deserialize(std::istringstream &iss, const size_t size);

private:
	Chunk(const size_t x, const size_t z) :
			x_(x), z_(z) {}

	const size_t x_, z_;
	Palette palette_;
	PackedArray<> terrain_{ kWidth * kWidth * kHeight };
};

} //namespace pgvoxel
