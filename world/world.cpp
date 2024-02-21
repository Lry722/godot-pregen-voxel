#include "world.h"

namespace pgvoxel
{
    static inline size_t globalIndex(const size_t x, const size_t z) {
        return x << 16 | z;
    }


} // namespace lry
