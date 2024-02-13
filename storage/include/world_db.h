#pragma once

#include <lmdb.h>
#include "world_config.h"

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

        ~WorldDB()
        {
            mdb_env_close(env_);
            delete instance_;
        }

    private:
        WorldDB();

        static inline WorldDB *instance_ = nullptr;

        MDB_env *env_{nullptr};
        MDB_dbi terrain_db_{}, chunks_db_{};
    };

} // namespace lry