#include "world_db.h"
#include "core/variant/variant.h"
#include "forward.h"
#include "scope_guard.h"
#include "serialize.h"
#include "chunk.inl"

#include "core/error/error_macros.h"
#include "core/string/print_string.h"

#define MDB_CALL(ERR_RETVAL, MDB_FUNC, ...)           \
	do {                                              \
		if (const auto err = MDB_FUNC(__VA_ARGS__)) { \
			ERR_PRINT(mdb_strerror(err));             \
			return ERR_RETVAL;                        \
		}                                             \
	} while (false)

namespace pgvoxel{

WorldDB::WorldDB() {
	// 打开数据库环境
	// const auto &config = WorldConfig::instance().data;
	MDB_CALL(, mdb_env_create, &env_);
	MDB_CALL(, mdb_env_set_maxdbs, env_, kMaxdbs);
	MDB_CALL(, mdb_env_set_mapsize, env_, kMapsize);
	MDB_CALL(, mdb_env_open, env_, kDatabaseEnv, MDB_NOSUBDIR, kPermission);

	// 打开各个数据库
	auto txn = beginTransaction(0);
	MDB_CALL(, mdb_dbi_open, txn, kTerrainDB, MDB_CREATE, &terrain_db_);
	MDB_CALL(, mdb_dbi_open, txn, kGenerationDB, MDB_CREATE, &generation_db_);
	MDB_CALL(, mdb_txn_commit, txn);

	print_verbose("Succeed opening database.")
}

std::unique_ptr<LoadedChunk> WorldDB::loadChunk(const Coord &pos) {
	// 依据chunk坐标构造指向地形数据的key，获取key所在位置的地形数据，反序列化数据
	MDB_val key, data;
	size_t key_data = pos.x << 16 | pos.z;
	key.mv_size = sizeof(size_t);
	key.mv_data = &key_data;

	{
		auto txn = beginTransaction(MDB_RDONLY);
		auto guard = scope_guard(&WorldDB::abortTransaction, this, txn);
		MDB_CALL(nullptr, mdb_get, txn, terrain_db_, &key, &data);
		commitTransaction(txn, guard);
	}

	std::istringstream iss({ static_cast<char *>(data.mv_data), data.mv_size });
	auto chunk = LoadedChunk::create(pos);
	iss >> *chunk;
	print_verbose(String("Succeed loading chunk {0}.").format(varray(toVector3i(chunk->position_))));
	return chunk;
}

void WorldDB::saveChunk(LoadedChunk *chunk) {
	// 依据chunk坐标构造指向地形数据的key，序列化地形数据，写入key所在位置
	MDB_val key, data;
	size_t key_data = chunk->position_.x << 16 | chunk->position_.z;
	key.mv_size = sizeof(size_t);
	key.mv_data = &key_data;
	std::ostringstream oss;
	oss << *chunk;
	std::string_view buffer{ oss.view() };
	data.mv_size = buffer.size();
	data.mv_data = const_cast<char *>(buffer.data());

	auto txn = beginTransaction(MDB_WRITEMAP | MDB_NOSYNC);
	auto guard = scope_guard(&WorldDB::abortTransaction, this, txn);
	MDB_CALL(, mdb_put, txn, terrain_db_, &key, &data, 0);
	commitTransaction(txn, guard);
	print_verbose(String("Succeed saving chunk {0}.").format(varray(toVector3i(chunk->position_))));
}

std::unique_ptr<GenerationChunk> WorldDB::loadGenerationChunk(const CoordAxis x, const CoordAxis z) {
	// 逻辑和loadChunk一样，只是操作的数据库是generation而不是terrain
	MDB_val key, data;
	size_t key_data = x << 16 | z;
	key.mv_size = sizeof(size_t);
	key.mv_data = &key_data;

	{
		auto txn = beginTransaction(MDB_RDONLY);
		auto guard = scope_guard(&WorldDB::abortTransaction, this, txn);
		MDB_CALL(nullptr, mdb_get, txn, generation_db_, &key, &data);
		commitTransaction(txn, guard);
	}

	std::istringstream iss({ static_cast<char *>(data.mv_data), data.mv_size });
	// chunk 类型也不一样
	auto chunk = GenerationChunk::create({x, 0, z});
	iss >> *chunk;
	print_verbose(String("Succeed loading chunk {0}.").format(varray(toVector3i(chunk->position_))))
	return chunk;
}
void WorldDB::saveGenerationChunk(GenerationChunk *chunk) {
	// 逻辑和saveChunk一样，只是操作的数据库是generation而不是terrain
	MDB_val key, data;
	size_t key_data = chunk->position_.x << 16 | chunk->position_.z;
	key.mv_size = sizeof(size_t);
	key.mv_data = &key_data;
	std::ostringstream oss;
	oss << *chunk;
	std::string_view buffer{ oss.view() };
	data.mv_size = buffer.size();
	data.mv_data = const_cast<char *>(buffer.data());

	auto txn = beginTransaction(MDB_WRITEMAP | MDB_NOSYNC);
	auto guard = scope_guard(&WorldDB::abortTransaction, this, txn);
	MDB_CALL(, mdb_put, txn, generation_db_, &key, &data, 0);
	commitTransaction(txn, guard);
	print_verbose(String("Succeed saving chunk {0}.").format(varray(toVector3i(chunk->position_))))
}

WorldDB::~WorldDB() {
	mdb_env_close(env_);
	delete instance_;
}

MDB_txn *WorldDB::beginTransaction(unsigned int flags) {
	MDB_txn *txn;
	MDB_CALL(nullptr, mdb_txn_begin, env_, nullptr, flags, &txn);
	return txn;
}

void WorldDB::commitTransaction(MDB_txn *txn, auto &guard) {
	MDB_CALL(, mdb_txn_commit, txn);
	guard.release();
}

void WorldDB::abortTransaction(MDB_txn *txn) {
	mdb_txn_abort(txn);
}

} //namespace pgvoxel::storage
