#include "chunk.h"
#include "serialize.h"
#include <sstream>

void lry::Chunk::serialize(std::ostringstream & oss)
{
    oss << terrain_palette_ << terrain_;
}

void lry::Chunk::deserialize(std::istringstream & iss, const size_t size)
{
    iss >> terrain_palette_ >> terrain_;
}
