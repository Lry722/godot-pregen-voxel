#pragma once
#include "typedefs.h"
#include <lmdb.h>

#define CONSTANT_STRING(name, str) static const char *name = str;

namespace lry
{
    namespace config
    {
        namespace filename
        {
            CONSTANT_STRING(kConfig, "config.yml")
        } // namespace filename
        
    } // namespace config
    
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
        static const size_t kChunkWidth = 16;
        static const size_t kChunkHeight = 256;
        static const size_t kChunkSize = kChunkWidth * kChunkWidth * kChunkHeight;
        static const size_t kMaxVoxelData = std::numeric_limits<VoxelData>::max();
    } // namespace storage
    
}