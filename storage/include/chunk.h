#pragma once

#include "typedefs.h"
#include "packed_array.h"
#include "palette.h"

#include <cstdint>
#include <glm/glm.hpp>

namespace lry
{
    class Chunk
    {
    public:
        friend class WorldDB;
        static std::unique_ptr<Chunk> create(const size_t x, const size_t z) {
            return std::unique_ptr<Chunk>(new Chunk(x, z));
        }

    public:
        // 应该使用智能指针来管理Chunk，不应该出现拷贝
        Chunk(const Chunk &other) = delete;
        Chunk &operator=(const Chunk &other) = delete;

        void setVoxel(const Vec3 pos, const VoxelData data);
        VoxelData getVoxel(const Vec3 pos) const;
        // 序列化
        void serialize(std::ostringstream &oss);
        // 反序列化
        void deserialize(std::istringstream &iss, const size_t size);

    private:
        Chunk(const size_t x, const size_t z) : x_(x), z_(z) {}
        
    private:
        const size_t x_, z_;
        Palette palette_;
        PackedArray<> terrain_{storage::kChunkSize};
    };

} // namespace lry
