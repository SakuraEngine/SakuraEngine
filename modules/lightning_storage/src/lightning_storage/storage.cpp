#include "SkrRT/platform/memory.h"
#include <SkrRT/platform/filesystem.hpp>
#include "SkrRT/misc/log.h"
#include "SkrRT/containers/string.hpp"
#include "SkrLightningStorage/storage.h"
#include "lmdb/lmdb.h"

SLightningEnvironmentId skr_lightning_storage_create_environment(const char8_t* name)
{
    std::error_code ec = {};
    if (!skr::filesystem::exists((const char*)name, ec))
    {
        SKR_LOG_INFO(u8"subdir %s not existed, create it", (const char*)name);
        skr::filesystem::create_directories(skr::filesystem::path((const char*)name), ec);
    }
    
    SLightningEnvironment* env = (SLightningEnvironment*)sakura_malloc(sizeof(SLightningEnvironment));
    mdb_env_create(&env->env);
    mdb_env_set_maxdbs(env->env, 50);
    mdb_env_set_mapsize(env->env, (size_t)1048576 * (size_t)16); // 1MB * 16
    if (const int rc = mdb_env_open(env->env, (const char*)name, 0, 0664)) 
    {
        skr::string err = (const char8_t*)mdb_strerror(rc);
        SKR_LOG_ERROR(u8"mdb_env_open failed: %d, %s", rc, err.c_str());
    }
    return env;
}

void skr_lightning_storage_free_environment(SLightningEnvironmentId environment)
{
    mdb_env_close(environment->env);
    sakura_free(environment);
}

SLightningStorageId skr_open_lightning_storage(SLightningEnvironmentId environment, const struct SLightningStorageOpenDescriptor* desc)
{
    const bool readonly = desc->flags & LIGHTNING_STORAGE_OPEN_READ_ONLY;
    auto env = environment->env;
    SLightningStorage* storage = (SLightningStorage*)sakura_malloc(sizeof(SLightningStorage));
    MDB_txn* txn;
    MDB_dbi dbi;
    if (const int rc = mdb_txn_begin(env, nullptr, readonly ? MDB_RDONLY : 0, &txn)) 
    {
        skr::string err = (const char8_t*)mdb_strerror(rc);
        SKR_LOG_ERROR(u8"mdb_txn_begin failed: %d, %s", rc, err.c_str());
    }
    auto dbi_flags = 0;
    if (desc->flags & LIGHTNING_STORAGE_OPEN_CREATE)
    {
        dbi_flags |= MDB_CREATE;
    }
    if (const int rc = mdb_dbi_open(txn, (const char*)desc->name, dbi_flags, &dbi)) 
    {
        skr::string err = (const char8_t*)mdb_strerror(rc);
        SKR_LOG_ERROR(u8"mdb_dbi_open failed: %d, %s", rc, err.c_str());
    }
    else if (!readonly)
    {
        if (const int rc = mdb_drop(txn, dbi, 0) )
        {
            skr::string err = (const char8_t*)mdb_strerror(rc);
            SKR_LOG_ERROR(u8"mdb_dbi_drop failed: %d, %s", rc, err.c_str());
        }
    }
    mdb_txn_commit(txn);
    storage->mdbi = dbi;
    storage->env = environment;
    return storage;
}

void skr_close_lightning_storage(SLightningStorageId storage)
{
    MDB_dbi dbi = static_cast<MDB_dbi>(storage->mdbi);
    mdb_dbi_close(storage->env->env, dbi);
    sakura_free(storage);
}
