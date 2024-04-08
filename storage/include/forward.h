#pragma once

#include "core/math/vector3i.h"
#include <glm/glm.hpp>

namespace pgvoxel {

typedef uint32_t CoordAxis;
typedef glm::vec<3, CoordAxis> Coord;
inline Vector3i toVector3i(const Coord &coord) {
	// 需要确保Coord中的值都不超过int的上限，因为Vector3i中存的是int不是uint
	return Vector3i(coord.x, coord.y, coord.z);
}
inline Coord toCoord(const Vector3i &vector3i) {
	return Coord(vector3i.x, vector3i.y, vector3i.z);
}
// 涉及到坐标转换时都必须按这里规定的zxy的顺序
inline size_t pos_to_index(const Coord pos, const size_t width_bits, const size_t height_bits) {
	return (pos.z << (width_bits + height_bits)) + (pos.x << (height_bits)) + pos.y;
}

static const size_t kLoadedChunkWidth = 32;
static const size_t kLoadedChunkHeight = 32;
static const size_t kGeneratingChunkWidth = 32;
static const size_t kGeneratingChunkHeight = 512;

typedef std::uint32_t VoxelData;
static const size_t kMaxVoxelData = std::numeric_limits<VoxelData>::max();

template <CoordAxis kWidth, CoordAxis kHeight>
class Chunk;
typedef Chunk<kLoadedChunkWidth, kLoadedChunkHeight> LoadedChunk;
typedef Chunk<kGeneratingChunkWidth, kGeneratingChunkHeight> GenerationChunk;

} //namespace pgvoxel
