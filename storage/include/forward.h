#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include "core/math/vector3i.h"

namespace pgvoxel {

typedef uint64_t CoordAxis;
typedef glm::vec<3, CoordAxis> Coord;

inline Vector3i toVector3i(const Coord &coord) { return Vector3i(coord.x, coord.y, coord.z); }
inline Coord toCoord(const Vector3i &vector3i) { return Coord(vector3i.x, vector3i.y, vector3i.z); }
// 涉及到坐标转换时都必须按这里规定的zxy的顺序
inline uint64_t pos_to_index(const Coord &pos, const uint8_t width_bits, const uint8_t height_bits) {
    return (pos.z << (width_bits + height_bits)) + (pos.x << (height_bits)) + pos.y;
}

// x, z 上限为 2^26 - 1, y 上限为 2^12 - 1
inline uint64_t global_pos_to_index(const Coord &pos) { return pos_to_index(pos, 26, 12); }

static const uint16_t kLoadedChunkWidth = 32;
static const uint16_t kLoadedChunkHeight = 32;
static const uint16_t kGeneratingChunkWidth = 32;
static const uint16_t kGeneratingChunkHeight = 512;

typedef std::uint32_t VoxelData;
static const VoxelData kMaxVoxelData = std::numeric_limits<VoxelData>::max();

template <CoordAxis kWidth, CoordAxis kHeight>
class Chunk;
typedef Chunk<kLoadedChunkWidth, kLoadedChunkHeight> LoadedChunk;
typedef Chunk<kGeneratingChunkWidth, kGeneratingChunkHeight> GenerationChunk;

}  // namespace pgvoxel

namespace std {
template <>
struct hash<glm::vec<3, uint64_t>> {
    uint64_t operator()(const pgvoxel::Coord &pos) const noexcept {
        return pgvoxel::global_pos_to_index(pos);
    }
};
}  // namespace std
