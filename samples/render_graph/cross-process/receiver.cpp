#include "platform/process.h"
#include "mdb_utils.h"
#include <ghc/filesystem.hpp>

int receiver_main(int argc, char* argv[])
{
    auto id = skr_get_current_process_id();
    SKR_LOG_DEBUG("exec_mode: %s, process id: %lld", argv[1], id);
    const SProcessId provider_id = std::stoll(argv[2]);
    SKR_LOG_INFO("provider id: %lld", provider_id);

    MDB_env* env = nullptr;
    env_create(&env);
    MDB_dbi dbi;
    dbi_create(env, &dbi, true);

    // Open txn
    {
        // ZoneScopedN("MDBTransaction");
        MDB_txn* parentTxn = nullptr;
        MDB_txn* txn;
        bool success = false;
        int c = 0;
        while (!success && c < 1000)
        {
            if (const int rc = mdb_txn_begin(env, parentTxn, MDB_RDONLY, &txn)) 
            {
                SKR_LOG_ERROR("mdb_txn_begin failed: %d", rc);
            }
            else
            {
                SKR_LOG_INFO("mdb_txn_begin succeed: %d", rc);
            }

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
            MDB_val key = {(size_t)keyString.size(), (void*)keyString.data()};
            MDB_val data;

            //Position the cursor, key and data are available in key
            if (int rc = mdb_cursor_get(cursor, &key, &data, MDB_SET_KEY)) 
            {
                //No value found
                SKR_LOG_INFO("query proc-links with key %s found no value: %d", keyString.c_str(), rc);
                mdb_cursor_close(cursor);
                success = false;
            }
            else
            {
                SProcessId what = *(SProcessId*)data.mv_data;
                SKR_LOG_INFO("query proc-links with key %lld found value: %lld", provider_id, what);
                mdb_cursor_close(cursor);
                success = true;
            }
            mdb_txn_commit(txn);
            mdb_txn_reset(txn);

            c++;
        }
    }
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return 0;
}