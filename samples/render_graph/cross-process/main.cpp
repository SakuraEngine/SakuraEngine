#include "platform/process.h"
#include "utils/log.h"
#include "lmdb/lmdb.h"
#include <ghc/filesystem.hpp>

#include "tracy/Tracy.hpp"

static const char* exec_name;

int main(int argc, char* argv[])
{
    exec_name = argv[0];
    if (argc == 1)
    {
        SKR_LOG_DEBUG("exec_name: %s", exec_name);
    
        const char* provider_arguments[] = { "provider", "-1"};
        auto provider = skr_run_process(exec_name, 
            provider_arguments, 2, "provider.log");
        auto provider_result = skr_wait_process(provider);

        const auto provider_id = skr_get_process_id(provider);
        eastl::string providerIdString = eastl::to_string(provider_id);
        const char* receiver_arguments[] = { "receiver", providerIdString.c_str() };
        auto receiver = skr_run_process(exec_name, 
            receiver_arguments, 2, "receiver.log");

        auto receriver_result = skr_wait_process(receiver);
        return receriver_result + provider_result;
    }
    else
    {
        auto id = skr_get_current_process_id();
        SKR_LOG_DEBUG("exec_mode: %s, process id: %lld", argv[1], id);
        auto is_receiver = (strcmp(argv[1], "receiver") == 0);
        const SProcessId provider_id = is_receiver ? std::stoll(argv[2]) : skr_get_current_process_id();

        if (!ghc::filesystem::exists("./cross-proc"))
        {
            SKR_LOG_INFO("subdir cross-proc not existed, create it");
            ghc::filesystem::create_directories(ghc::filesystem::path("./cross-proc"));
        }

        MDB_env *env;
        {
            // ZoneScopedN("MDBInitializeEnv");

            if (const int rc = mdb_env_create(&env)) 
            {
                SKR_LOG_ERROR("mdb_env_create failed: %d", rc);
            }
            mdb_env_set_maxdbs(env, 50);
            mdb_env_set_mapsize(env, (size_t)1048576 * (size_t)16); // 1MB * 16
            if (const int rc = mdb_env_open(env, "./cross-proc", 0, 0664)) 
            {
                SKR_LOG_ERROR("mdb_env_open failed: %d", rc);
            }
            else
            {
                SKR_LOG_INFO("mdb_env_open succeed: %d", rc);
            }
        }
        MDB_dbi dbi;
        // Open txn
        {
            // ZoneScopedN("MDBTransaction");
            MDB_txn* parentTxn = nullptr;
            MDB_txn* txn;
            if (const int rc = mdb_txn_begin(env, parentTxn, is_receiver ? MDB_RDONLY : 0, &txn)) 
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

                auto dbi_flags = MDB_DUPSORT | MDB_CREATE;
                if (const int rc = mdb_dbi_open(txn, "proc-links", dbi_flags, &dbi)) 
                {
                    SKR_LOG_ERROR("mdb_dbi_open failed: %d", rc);
                }
            }
            mdb_txn_commit(txn);
        }
        // Open txn
        {
            // ZoneScopedN("MDBTransaction");
            MDB_txn* parentTxn = nullptr;
            MDB_txn* txn;
            if (const int rc = mdb_txn_begin(env, parentTxn, is_receiver ? MDB_RDONLY : 0, &txn)) 
            {
                SKR_LOG_ERROR("mdb_txn_begin failed: %d", rc);
            }
            else
            {
                SKR_LOG_INFO("mdb_txn_begin succeed: %d", rc);
            }
            // Txn body: write db
            if (!is_receiver)
            {
                //Initialize the key with the key we're looking for
                eastl::string keyString = eastl::to_string(provider_id);
                MDB_val key = {(size_t)keyString.size(), (void *)keyString.data()};
                MDB_val data = {sizeof(provider_id), (void *)&provider_id};

                if (int rc = mdb_put(txn, dbi, &key, &data, 0))
                {
                    SKR_LOG_ERROR("mdb_put failed: %d", rc);
                }
                else
                {
                    SKR_LOG_INFO("mdb_put succeed: %d, key: %s, val: %lld", rc, keyString.c_str(), provider_id);
                }
            }
            else
            {
                MDB_cursor *cursor;
                if (int rc = mdb_cursor_open(txn, dbi, &cursor)) 
                {
                    SKR_LOG_ERROR("mdb_cursor_open failed: %d", rc);
                }
                else
                {
                    SKR_LOG_INFO("mdb_cursor_open succeed: %d", rc);
                }

                //Initialize the key with the key we're looking for
                eastl::string keyString = eastl::to_string(provider_id);
                MDB_val key = {(size_t)keyString.size(), (void *)keyString.data()};
                MDB_val data;

                //Position the cursor, key and data are available in key
                if (int rc = mdb_cursor_get(cursor, &key, &data, MDB_SET_RANGE)) 
                {
                    //No value found
                    SKR_LOG_INFO("query proc-links with key %lld found no value: %d", id, rc);
                    mdb_cursor_close(cursor);
                }
                else
                {
                    //Position the curser at the next position
                    mdb_cursor_get(cursor, &key, &data, MDB_NEXT);
                    mdb_cursor_close(cursor);
                }
            }
            mdb_txn_commit(txn);
        }
        mdb_dbi_close(env, dbi);
        mdb_env_close(env);
    }

    return 0;
}