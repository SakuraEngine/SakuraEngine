#include "SkrRT/misc/log.h"
#include "SkrLightningStorage/mdb.h"

#include "SkrTestFramework/framework.hpp"

struct MDBTests
{

};

TEST_CASE_METHOD(MDBTests, "Environment")
{
    auto env = skr_lightning_storage_create_environment(u8"./test_mdb");
    EXPECT_NE(env, nullptr);
    skr_lightning_storage_free_environment(env);
}

TEST_CASE_METHOD(MDBTests, "Storage")
{
    auto env = skr_lightning_storage_create_environment(u8"./test_mdb");
    EXPECT_NE(env, nullptr);

    SLightningStorageOpenDescriptor desc = {};
    desc.flags = LIGHTNING_STORAGE_OPEN_CREATE;
    desc.name = u8"test_storage";
    auto storage = skr_open_lightning_storage(env, &desc);
    EXPECT_NE(storage, nullptr);
    
    skr_close_lightning_storage(storage);
    skr_lightning_storage_free_environment(env);
}

TEST_CASE_METHOD(MDBTests, "TXN")
{
    auto env = skr_lightning_storage_create_environment(u8"./test_mdb");
    EXPECT_NE(env, nullptr);

    SLightningStorageOpenDescriptor desc = {};
    desc.flags = LIGHTNING_STORAGE_OPEN_CREATE;
    desc.name = u8"test_storage";
    auto storage = skr_open_lightning_storage(env, &desc);
    EXPECT_NE(storage, nullptr);
    
    if (auto txn = skr_open_lightning_transaction(env, nullptr, 0))
    {
        skr::string key_string = u8"hello";
        skr::string value_string = u8"world";

        SLightningStorageValue key = { key_string.size() + 1, key_string.c_str() };
        SLightningStorageValue value = { value_string.size() + 1, value_string.c_str() };

        EXPECT_TRUE(skr_lightning_storage_write(txn, storage, &key, &value));

        SLightningStorageValue readed_value;
        EXPECT_TRUE(skr_lightning_storage_read(txn, storage, &key, &readed_value));

        skr::string readed_string = (const char8_t*)readed_value.data;
        EXPECT_EQ(readed_string, value_string);

        EXPECT_TRUE(skr_lightning_storage_del(txn, storage, &key));
        EXPECT_FALSE(skr_lightning_storage_read(txn, storage, &key, &readed_value));

        EXPECT_TRUE(skr_commit_lightning_transaction(txn));
    }

    skr_close_lightning_storage(storage);
    skr_lightning_storage_free_environment(env);
}

TEST_CASE_METHOD(MDBTests, "TXN2")
{
    auto env = SLightningEnvironment::Open(u8"./test_mdb");
    EXPECT_NE(env, nullptr);

    SLightningStorageOpenDescriptor desc = {};
    desc.flags = LIGHTNING_STORAGE_OPEN_CREATE;
    desc.name = u8"test_storage";
    auto storage = env->open_storage(&desc);
    EXPECT_NE(storage, nullptr);
    
    if (auto txn = env->open_transaction(nullptr, 0))
    {
        skr::string key_string = u8"hello";
        skr::string value_string = u8"world";

        SLightningStorageValue key = { key_string.size() + 1, key_string.c_str() };
        SLightningStorageValue value = { value_string.size() + 1, value_string.c_str() };

        EXPECT_TRUE(storage->write(txn, key, value));

        SLightningStorageValue readed_value = storage->read(txn, key);
        skr::string readed_string = (const char8_t*)readed_value.data;
        EXPECT_EQ(readed_string, value_string);

        EXPECT_TRUE(storage->del(txn, key));

        EXPECT_TRUE(env->commit_transaction(txn));
    }

    env->close_storage(storage);
    SLightningEnvironment::Free(env);
}
