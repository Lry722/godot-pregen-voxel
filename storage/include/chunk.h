#pragma once

#include "packed_array.h"
#include "palette.h"
#include "typedefs.h"
#include <cstdint>

namespace lry
{
    class Chunk
    {
        friend class WorldDB;

    public:
        Chunk(const std::uint32_t x, const std::uint32_t z) : x_(x), z_(z) {}
        // 应该使用智能指针来管理Chunk，不应该出现拷贝
        Chunk(const Chunk &other) = delete;
        Chunk &operator=(const Chunk &other) = delete;

        void setVoxel();

        // 序列化
        void serialize(std::ostringstream & oss);
        // 反序列化
        void deserialize(std::istringstream & iss, const size_t size);

    private:
        const std::uint32_t x_, z_;
        Palette<ChunkPaletteKey, ChunkPaletteData> terrain_palette_{};
        PackedArray<> terrain_{};
    };

} // namespace lry
