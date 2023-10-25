#include "SkrLightningStorage/storage.h"

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
