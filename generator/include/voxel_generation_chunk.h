#pragma once

#include "core/object/ref_counted.h"
#include "world_db.h"
#include "chunk.inl"
#include <memory>

namespace pgvoxel{

class VoxelGeneratorLayer;

class VoxelGenerationChunk : public RefCounted {
	GDCLASS(VoxelGenerationChunk, RefCounted)
public:
	VoxelGenerationChunk() = default;
	VoxelGenerationChunk(int32_t x, int32_t z, VoxelGeneratorLayer *layer);

	// 设置一个点的值
	void setVoxel(const Vector3i pos, const VoxelData data, int layer);
	// 获取一个点的值
	VoxelData getVoxel(const Vector3i pos, int layer) const;
	// 设置位于(x, z)处，从buttom到top间的长条的值，在修改大量值时效率高于逐个调用setVoxel
	void setBar(const int32_t x, const int32_t z, const int32_t buttom, const int32_t top, const VoxelData data, int layer);
	// 设置begin到end两点围成的区域中的值，效果等同于遍历水平面，逐个调用setBar
	void setBlock(const Vector3i begin, const Vector3i end, const VoxelData data, int layer);

	// 区块的坐标是只读的
	int32_t getX() const;
	int32_t getZ() const;

	void save() {
		WorldDB::singleton().saveGenerationChunk(data_.get());
	}

private:
	static void _bind_methods();

	bool initialized_{ false };
	VoxelGeneratorLayer *layer_{ nullptr };
	std::unique_ptr<GenerationChunk> data_;
};

} //namespace pgvoxel::generator
