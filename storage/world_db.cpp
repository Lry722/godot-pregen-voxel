#include "world_db.h"
#include "constants.h"
#include <spdlog/spdlog.h>

#define MDB_CALL(ERR_EXP, MDB_FUNC, ...)                                    \
    do                                                                      \
    {                                                                       \
        if (const auto err = MDB_FUNC(__VA_ARGS__))                         \
        {                                                                   \
            spdlog::error("Error opening database: {}", mdb_strerror(err)); \
            ERR_EXP;                                                        \
        }                                                                   \
    } while (0)

namespace lry
{

    WorldDB::WorldDB()
    {
        using namespace db;

        spdlog::info("Start opening database.");

        const auto &config = WorldConfig::instance().data;
        MDB_CALL(return, mdb_env_create, &env_);
        MDB_CALL(return, mdb_env_set_maxdbs, env_, 8);
        MDB_CALL(return, mdb_env_set_mapsize, env_, config.width * config.width);
        MDB_CALL(return, mdb_env_open, env_, filename::kDatabaseEnv, MDB_NOSUBDIR, kPermission);

        MDB_txn *txn{};
        MDB_CALL(return, mdb_txn_begin, env_, nullptr, 0, &txn);
        MDB_CALL(return, mdb_dbi_open, txn, filename::kTerrainDB, MDB_CREATE, &terrain_db_);
        MDB_CALL(return, mdb_txn_commit, txn);
        
        spdlog::info("Succeed opening database.");
    }

} // namespace lry
