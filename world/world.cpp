#include "world.h"

namespace pgvoxel
{
    static size_t globalIndex(const size_t x, const size_t z) {
        return x << 16 | z;
    }


} // namespace lry
