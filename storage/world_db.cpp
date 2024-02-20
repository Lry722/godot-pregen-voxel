#include "world_db.h"
#include "chunk.h"
#include "constants.h"
#include "scope_guard.h"
#include "serialize.h"
#include "world_config.h"

#define MDB_CALL(ERR_EXP, MDB_FUNC, ...)                        \
	do {                                                        \
		if (const auto err = MDB_FUNC(__VA_ARGS__)) {           \
			/* spdlog::error("LMDB: {}", mdb_strerror(err)); */ \
			ERR_EXP;                                            \
		}                                                       \
	} while (false)

namespace pgvoxel {

WorldDB::WorldDB() {
	using namespace db;

	// spdlog::info("Start opening database.");

	// 打开数据库环境
	const auto &config = WorldConfig::instance().data;
	MDB_CALL(return, mdb_env_create, &env_);
	MDB_CALL(return, mdb_env_set_maxdbs, env_, kMaxdbs);
	MDB_CALL(return, mdb_env_set_mapsize, env_, kMapsize);
	MDB_CALL(return, mdb_env_open, env_, filename::kDatabaseEnv, MDB_NOSUBDIR, kPermission);

	// 打开各个数据库
	MDB_txn *txn{};
	MDB_CALL(return, mdb_txn_begin, env_, nullptr, 0, &txn);
	MDB_CALL(return, mdb_dbi_open, txn, filename::kTerrainDB, MDB_CREATE, &terrain_db_);
	MDB_CALL(return, mdb_txn_commit, txn);

	// spdlog::info("Succeed opening database.");
}

std::unique_ptr<LoadedChunk> WorldDB::loadChunk(const size_t x, const size_t z) {
	// 依据chunk坐标构造指向地形数据的key，获取key所在位置的地形数据，反序列化数据
	// spdlog::info("Start loading chunk({}, {}).", x, z);
	MDB_val key, data;
	size_t key_data = x << 16 | z;
	key.mv_size = sizeof(size_t);
	key.mv_data = &key_data;

	{
		auto txn = beginTransaction(MDB_RDONLY);
		auto guard = scope_guard(&WorldDB::abortTransaction, this, txn);
		MDB_CALL(return nullptr, mdb_get, txn, terrain_db_, &key, &data);
		commitTransaction(txn, guard);
	}

	std::istringstream iss({ static_cast<char *>(data.mv_data), data.mv_size });
	auto chunk = LoadedChunk::create(x, z);
	iss >> *chunk;
	// spdlog::info("Succeed loading chunk({}, {}).", x, z);
	return chunk;
}

void WorldDB::saveChunk(LoadedChunk *chunk) {
	// 依据chunk坐标构造指向地形数据的key，序列化地形数据，写入key所在位置
	// spdlog::info("Start saving chunk({}, {}).", chunk->x_, chunk->z_);
	MDB_val key, data;
	size_t key_data = chunk->x_ << 16 | chunk->z_;
	key.mv_size = sizeof(size_t);
	key.mv_data = &key_data;
	std::ostringstream oss;
	oss << *chunk;
	std::string_view buffer{ oss.view() };
	data.mv_size = buffer.size();
	data.mv_data = const_cast<char *>(buffer.data());

	auto txn = beginTransaction(MDB_WRITEMAP | MDB_NOSYNC);
	auto guard = scope_guard(&WorldDB::abortTransaction, this, txn);
	MDB_CALL(return, mdb_put, txn, terrain_db_, &key, &data, 0);
	commitTransaction(txn, guard);
	// spdlog::info("Succeed saving chunk({}, {}).", chunk->x_, chunk->z_);
}

WorldDB::~WorldDB() {
	mdb_env_close(env_);
	delete instance_;
}

MDB_txn *WorldDB::beginTransaction(unsigned int flags) {
	MDB_txn *txn;
	MDB_CALL(return nullptr, mdb_txn_begin, env_, nullptr, flags, &txn);
	return txn;
}

void WorldDB::commitTransaction(MDB_txn *txn, auto &guard) {
	MDB_CALL(return, mdb_txn_commit, txn);
	guard.release();
}

void WorldDB::abortTransaction(MDB_txn *txn) {
	mdb_txn_abort(txn);
}

} //namespace pgvoxel
