#include "voxel_generation_chunk.h"
#include "chunk.inl"
#include "core/string/print_string.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "forward.h"
#include "voxel_generator_layer.h"
#include "world_config.h"
#include "world_db.h"

#include "core/object/object.h"

#include <glm/fwd.hpp>
#include <memory>

namespace pgvoxel {

VoxelGenerationChunk::VoxelGenerationChunk(int32_t x, int32_t z, VoxelGeneratorLayer *layer) :
		layer_(layer) {
	if (layer->getIndex() == 0) {
		// 第一层创建空区块作为数据
		data_ = GenerationChunk::create({ x, 0, z });
	} else {
		// 否则从generation db中读取之前缓存的生成结果作为数据
		data_ = WorldDB::singleton().loadGenerationChunk(x, z);
	}
	initialized_ = true;
}

void VoxelGenerationChunk::setVoxel(const Vector3i pos, const VoxelData data) {
	data_->setVoxel(toCoord(pos), data);
}

VoxelData VoxelGenerationChunk::getVoxel(const Vector3i pos) const {
	if (pos.x >= 0 && pos.y >= 0 && pos.z >= 0 &&
			pos.x < static_cast<int32_t>(kGeneratingChunkWidth) && pos.y < static_cast<int32_t>(kGeneratingChunkHeight) && pos.z < static_cast<int32_t>(kGeneratingChunkWidth)) {
		// 当访问当前区块中的格子时，直接返回对应的数据
		// print_line(String("Get {0}").format(varray(pos)));
		VoxelData result = data_->getVoxel(toCoord(pos));
		return result;
	} else {
		// // 当访问的格子超出了当前区块时，要找到所处的区块，返回所处区块中对应的数据
		// GET_WORLD_CONFIG(0, config);
		// int32_t world_x = getX() * config.width + pos.x;
		// int32_t world_z = getZ() * config.width + pos.z;
		// // 这里没有范围检查，需要用户自己确保传入的pos不超出世界范围
		// return layer_->getCachedChunk(world_x / config.width, world_z / config.width)->getVoxel(Coord(world_x % config.width, pos.y, world_z % config.width));
		print_error("VoxelGenerationChunk::getVoxel pos out of range");
		return 0;
	}
}

void VoxelGenerationChunk::setBar(const int32_t x, const int32_t z, const int32_t buttom, const int32_t top, const VoxelData data) {
	data_->setBar(x, z, buttom, top, data);
}

void VoxelGenerationChunk::setBlock(const Vector3i begin, const Vector3i end, const VoxelData data) {
	data_->setBlock(toCoord(begin), toCoord(end), data);
}

int32_t VoxelGenerationChunk::getX() const {
	return data_->position().x;
}

int32_t VoxelGenerationChunk::getZ() const {
	return data_->position().z;
}

void VoxelGenerationChunk::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_voxel", "pos", "data"), &VoxelGenerationChunk::setVoxel);
	ClassDB::bind_method(D_METHOD("get_voxel", "pos"), &VoxelGenerationChunk::getVoxel);
	ClassDB::bind_method(D_METHOD("set_bar", "x", "z", "buttom", "top", "data"), &VoxelGenerationChunk::setBar);
	ClassDB::bind_method(D_METHOD("set_block", "begin", "end", "data"), &VoxelGenerationChunk::setBlock);

	ClassDB::bind_method(D_METHOD("x"), &VoxelGenerationChunk::getX);
	ClassDB::bind_method(D_METHOD("z"), &VoxelGenerationChunk::getZ);
}

} //namespace pgvoxel
