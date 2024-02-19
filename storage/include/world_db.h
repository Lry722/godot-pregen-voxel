#pragma once

#include "chunk.h"

#include <lmdb.h>
#include <memory>
#include <cstdint>


namespace lry
{
    class WorldDB
    {

    public:
        static WorldDB &instance()
        {
            if (!instance_)
            {
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
        MDB_txn * beginTransaction(unsigned int flags);
        // 提交事务，并释放scope_guard
        void commitTransaction(MDB_txn * txn, auto &guard);
        // 中断事务
        void abortTransaction(MDB_txn * txn);

        static inline WorldDB *instance_ = nullptr;

        MDB_env *env_{nullptr};
        MDB_dbi terrain_db_{};
    };

} // namespace lry