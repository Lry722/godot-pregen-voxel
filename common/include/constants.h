#pragma once

#include <lmdb.h>
#include <cstdint>
#include <glm/vec3.hpp>

#define CONSTANT_STRING(name, str) static const char *name = str;
#define STRING_NAME(str) static const StringName str{ #str, true };

namespace pgvoxel {
// ------
// common
// ------

typedef std::uint32_t size_t;
typedef std::uint32_t VoxelData;
typedef glm::u32vec3 Vec3;

// ---------
// non common
// ---------

namespace config {

CONSTANT_STRING(kConfig, "config.yml")
CONSTANT_STRING(kBadConfig, "The world config was not loaded correctly.")

} //namespace config

namespace db {

// database names
CONSTANT_STRING(kDatabaseEnv, "world.db")
CONSTANT_STRING(kTerrainDB, "terrain")
CONSTANT_STRING(kGenerationDB, "generation")

// database enviroment paramters
static const MDB_dbi kMaxdbs = 4;
static const ::size_t kMapsize = 1073741824;
static const mdb_mode_t kPermission = 0664;

} //namespace db

namespace storage {
static const size_t kLoadedChunkWidth = 32;
static const size_t kLoadedChunkHeight = 32;
static const size_t kGeneratingChunkWidth = 128;
static const size_t kGeneratingChunkHeight = 512;
static const size_t kMaxVoxelData = std::numeric_limits<VoxelData>::max();

template <size_t kWidth, size_t kHeight>
class Chunk;
typedef Chunk<storage::kLoadedChunkWidth, storage::kLoadedChunkHeight> LoadedChunk;

} //namespace storage

namespace generator {

// STRING_NAME(generation_finished)
// STRING_NAME(chunk_generation_finished)

CONSTANT_STRING(generation_finished, "generation_finished")
CONSTANT_STRING(chunk_generation_finished, "chunk_generation_finished")

typedef storage::Chunk<storage::kGeneratingChunkWidth, storage::kGeneratingChunkHeight> GenerationChunk;

} //namespace generator

namespace world {

static const uint8_t kMaxLoadLevel = 8;
typedef uint8_t LoadLevel;
typedef LoadLevel LoadLevels[kMaxLoadLevel];

} //namespace world

} //namespace pgvoxel
