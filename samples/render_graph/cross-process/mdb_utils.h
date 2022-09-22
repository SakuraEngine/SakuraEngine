#pragma once
#include "utils/log.h"
#include "lmdb/lmdb.h"

inline static void env_create(MDB_env** penv)
{
    if (const int rc = mdb_env_create(penv)) 
    {
        SKR_LOG_ERROR("mdb_env_create failed: %d", rc);
    }
    mdb_env_set_maxdbs(*penv, 50);
    mdb_env_set_mapsize(*penv, (size_t)1048576 * (size_t)16); // 1MB * 16
    if (const int rc = mdb_env_open(*penv, "./cross-proc", 0, 0664)) 
    {
        SKR_LOG_ERROR("mdb_env_open failed: %d", rc);
    }
    else
    {
        SKR_LOG_INFO("mdb_env_open succeed: %d", rc);
    }
}

inline static void dbi_create(MDB_env* env, MDB_dbi* pdb, bool readonly)
{
    // Open txn
    MDB_txn* txn;
    if (const int rc = mdb_txn_begin(env, nullptr, readonly ? MDB_RDONLY : 0, &txn)) 
    {
        SKR_LOG_ERROR("mdb_txn_begin failed: %d", rc);
    }
    else
    {
        SKR_LOG_INFO("mdb_txn_begin succeed: %d", rc);
    }
    // Txn body: open db
    {
        // ZoneScopedN("MDBQuery");

        auto dbi_flags = MDB_CREATE;
        if (const int rc = mdb_dbi_open(txn, "proc-links", dbi_flags, pdb)) 
        {
            SKR_LOG_ERROR("mdb_dbi_open failed: %d", rc);
        }
        else if (!readonly)
        {
            if (const int rc = mdb_drop(txn, *pdb, 0) )
            {
                SKR_LOG_ERROR("mdb_dbi_drop failed: %d", rc);
            }
        }
    }
    mdb_txn_commit(txn);
}