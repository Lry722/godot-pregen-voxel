#pragma once

#include "core/string/string_name.h"

#include <lmdb.h>
#include <cstdint>
#include <glm/vec3.hpp>

#define CONSTANT_STRING(name, str) static const char *name = str;
#define STRING_NAME(str) static const StringName str{#str, true};

namespace pgvoxel {
// --------
// typedefs
// --------

typedef std::uint32_t size_t;
typedef std::uint32_t VoxelData;
typedef glm::u32vec3 Vec3;

// ---------
// constants
// ---------

namespace config {
namespace filename {
CONSTANT_STRING(kConfig, "config.yml")
}
CONSTANT_STRING(kBadConfig, "The world config was not loaded correctly.")
} //namespace config

namespace db {
namespace filename {
CONSTANT_STRING(kDatabaseEnv, "world.db")
CONSTANT_STRING(kTerrainDB, "terrain")
CONSTANT_STRING(kGenerationDB, "generation")
} //namespace filename
static const MDB_dbi kMaxdbs = 4;
static const ::size_t kMapsize = 1073741824;
static const mdb_mode_t kPermission = 0664;
} //namespace db

namespace storage {
static const size_t kLoadedChunkWidth = 16;
static const size_t kGeneratingChunkWidth = 16;
static const size_t kChunkHeight = 256;
static const size_t kMaxVoxelData = std::numeric_limits<VoxelData>::max();

} //namespace storage

namespace generator {

// STRING_NAME(generation_finished)
// STRING_NAME(chunk_generation_finished)

CONSTANT_STRING(generation_finished, "generation_finished")
CONSTANT_STRING(chunk_generation_finished, "chunk_generation_finished")

} //namespace generator

// --------
// template typedefs
// --------
template <size_t kWidth, size_t kHeight>
class Chunk;

typedef Chunk<storage::kLoadedChunkWidth, storage::kChunkHeight> LoadedChunk;
typedef Chunk<storage::kGeneratingChunkWidth, storage::kChunkHeight> GenerationChunk;
} //namespace pgvoxel
