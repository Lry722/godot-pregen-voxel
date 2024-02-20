#pragma once
#include "constants.h"
#include <sstream>

namespace pgvoxel
{
    template <typename T>
    std::ostringstream &operator<<(std::ostringstream &oss, T &p)
    {
        size_t size{0};
        oss.write(reinterpret_cast<char *>(&size), sizeof(size));
        const auto start_pos = oss.tellp();
        p.serialize(oss);
        const auto end_pos = oss.tellp();
        size = static_cast<size_t>(end_pos - start_pos);
        oss.seekp(start_pos - sizeof(size));
        oss.write(reinterpret_cast<char *>(&size), sizeof(size));
        oss.seekp(end_pos);

        return oss;
    }

    template <typename T>
    std::istringstream &operator>>(std::istringstream &iss, T &p)
    {
        size_t size;
        iss.read(reinterpret_cast<char *>(&size), sizeof(size_t));
        p.deserialize(iss, size);

        return iss;
    }
} // namespace pgvoxel
