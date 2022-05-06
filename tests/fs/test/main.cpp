#include "gtest/gtest.h"
#include "platform/vfs.h"
#include <string>
#include <iostream>
#include <ghc/filesystem.hpp>

class FSTest : public ::testing::Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_F(FSTest, mount)
{
    skr_vfs_desc_t fs_desc = {};
    fs_desc.app_name = "fs-test";
    fs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto fs = skr_create_vfs(&fs_desc);
    EXPECT_NE(fs, nullptr);
    EXPECT_NE(fs->mount_dir, nullptr);

    skr_vfs_desc_t abs_fs_desc = {};
    abs_fs_desc.app_name = "fs-test";
    abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    auto abs_fs = skr_create_vfs(&abs_fs_desc);
    EXPECT_NE(abs_fs, nullptr);
    EXPECT_EQ(abs_fs->mount_dir, nullptr);

    skr_free_vfs(fs);
    skr_free_vfs(abs_fs);
}

TEST_F(FSTest, read)
{
    skr_vfs_desc_t abs_fs_desc = {};
    abs_fs_desc.app_name = "fs-test";
    abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    const auto current_path = ghc::filesystem::current_path();
    abs_fs_desc.override_mount_dir = current_path.c_str();
    auto abs_fs = skr_create_vfs(&abs_fs_desc);
    EXPECT_NE(abs_fs, nullptr);
    EXPECT_NE(abs_fs->mount_dir, nullptr);
    skr_vfile_t f = {};
    abs_fs->procs.fopen(abs_fs, "./fs-test", SKR_FM_READ, nullptr, &f);

    skr_free_vfs(abs_fs);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}