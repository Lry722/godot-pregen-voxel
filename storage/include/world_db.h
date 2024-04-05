#pragma once

#include "chunk.h"

#include <lmdb.h>
#include <memory>

namespace pgvoxel {
class WorldDB {
public:
	static WorldDB &singleton() {
		if (!instance_) [[unlikely]] {
			instance_ = new WorldDB();
		}
		return *instance_;
	}

	// TODO: 或许应该把这些业务逻辑拆分到其他类中
	std::unique_ptr<LoadedChunk> loadChunk(const Coord &pos);
	void saveChunk(LoadedChunk *chunk);
	std::unique_ptr<GenerationChunk> loadGenerationChunk(const CoordAxis x, const CoordAxis z);
	void saveGenerationChunk(GenerationChunk *chunk);

	~WorldDB();

private:
	// database names
	// #define CONSTANT_STRING(name, str) inline static const char *name = str;
	inline static const char *kDatabaseEnv = "world.db";
	inline static const char *kTerrainDB = "terrain";
	inline static const char *kGenerationDB = "generation";

	// database enviroment paramters
	static const MDB_dbi kMaxdbs = 4;
	static const ::size_t kMapsize = 1073741824;
	static const mdb_mode_t kPermission = 0664;

private:
	WorldDB();
	// 开始新事务
	MDB_txn *beginTransaction(unsigned int flags);
	// 提交事务，并释放scope_guard
	void commitTransaction(MDB_txn *txn, auto &guard);
	// 中断事务
	void abortTransaction(MDB_txn *txn);

	static inline WorldDB *instance_ = nullptr;

	MDB_env *env_{};
	MDB_dbi terrain_db_{}, generation_db_{};
};

} //namespace pgvoxel
