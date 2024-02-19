#pragma once
#include <lmdb.h>
#include <cstdint>
#include <glm/vec3.hpp>

#define CONSTANT_STRING(name, str) static const char *name = str;

namespace lry
{
    // --------
    // typedefs
    // --------

    typedef std::uint32_t size_t;
    typedef std::uint32_t VoxelData;
    typedef glm::u32vec3 Vec3;

    // ---------
    // constants
    // ---------

    namespace config
    {
        namespace filename
        {
            CONSTANT_STRING(kConfig, "config.yml")
        }
    }

    namespace db
    {
        namespace filename
        {
            CONSTANT_STRING(kDatabaseEnv, "world.db")
            CONSTANT_STRING(kTerrainDB, "terrain")
        }
        static const MDB_dbi kMaxdbs = 4;
        static const ::size_t kMapsize = 1073741824;
        static const mdb_mode_t kPermission = 0664;
    }

    namespace storage
    {
        static const size_t kLoadedChunkWidth = 16;
        static const size_t kGeneratingChunkWidth = 16;
        static const size_t kChunkHeight = 256;
        static const size_t kMaxVoxelData = std::numeric_limits<VoxelData>::max();

    }

    // --------
    // template typedefs
    // --------
    template <size_t kWidth, size_t kHeight>
    class Chunk;

    typedef Chunk<storage::kLoadedChunkWidth, storage::kChunkHeight> LoadedChunk;
    typedef Chunk<storage::kGeneratingChunkWidth, storage::kChunkHeight> GeneratingChunk;
}