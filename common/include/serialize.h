#pragma once
#include <sstream>

template <typename T>
std::ostringstream &operator<<(std::ostringstream &oss, T &p)
{
    std::streampos start_pos = oss.tellp();
    oss.seekp(start_pos + sizeof(size_t));
    p.serialize(oss);
    size_t size{oss.tellp() - start_pos + sizeof(size_t)};
    oss.seekp(start_pos);
    oss.write(reinterpret_cast<char*>(&size), sizeof(size));

    return oss;
}

template <typename T>
std::istringstream &operator>>(std::istringstream &iss, T &p)
{
    size_t size;
    iss.read(reinterpret_cast<char*>(&size), sizeof(size_t));
    p.deserialize(iss, size);

    return iss;
}