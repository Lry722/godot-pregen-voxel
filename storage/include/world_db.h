#pragma once

#include "chunk.h"

#include <lmdb.h>
#include <cstdint>
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

	std::unique_ptr<LoadedChunk> loadChunk(const size_t x, const size_t z);
	void saveChunk(LoadedChunk *chunk);
	~WorldDB();

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
