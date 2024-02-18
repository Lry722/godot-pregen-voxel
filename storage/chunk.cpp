#include "chunk.h"
#include "serialize.h"
#include <sstream>

namespace lry
{
    static size_t vec3_to_index(const Vec3 position) {
        return position.z << 8 | position.x << 4 | position.y;
    }

    void Chunk::setVoxel(const Vec3 pos, const VoxelData new_data)
    {
        auto old_data = terrain_[vec3_to_index(pos)];
        if (old_data == new_data)
            return;

        if (palette_.update(new_data, old_data) && palette_.size() > terrain_.elementSize())
            terrain_.grow();

        old_data = new_data;
    }

    VoxelData Chunk::getVoxel(const Vec3 pos) const
    {
        return palette_.pick(terrain_[vec3_to_index(pos)]);
    }

    void Chunk::serialize(std::ostringstream &oss)
    {
        oss << palette_ << terrain_;
    }

    void Chunk::deserialize(std::istringstream &iss, const size_t size)
    {
        iss >> palette_ >> terrain_;
    }
} // namespace lry
