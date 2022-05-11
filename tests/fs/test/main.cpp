#include "gtest/gtest.h"
#include "platform/vfs.h"
#include <string>
#include <iostream>
#include <ghc/filesystem.hpp>
#include "utils/io.hpp"

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
    const auto current_path = ghc::filesystem::current_path().u8string();
    EXPECT_EQ(std::string(abs_fs->mount_dir), current_path);

    skr_free_vfs(fs);
    skr_free_vfs(abs_fs);
}

TEST_F(FSTest, readwrite)
{
    skr_vfs_desc_t abs_fs_desc = {};
    abs_fs_desc.app_name = "fs-test";
    abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    const auto current_path = ghc::filesystem::current_path().u8string();
    abs_fs_desc.override_mount_dir = current_path.c_str();
    auto abs_fs = skr_create_vfs(&abs_fs_desc);
    EXPECT_NE(abs_fs, nullptr);
    EXPECT_NE(abs_fs->mount_dir, nullptr);
    auto f = skr_vfs_fopen(abs_fs, "testfile",
    SKR_FM_READ_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
    const char8_t* string = u8"Hello, World!";
    skr_vfs_fwrite(f, string, 0, strlen(string));
    char8_t string_out[256];
    std::memset((void*)string_out, 0, 256);
    skr_vfs_fread(f, string_out, 0, strlen(string));
    EXPECT_EQ(std::string(string_out), std::string(u8"Hello, World!"));
    EXPECT_EQ(skr_vfs_fsize(f), strlen(string));
    EXPECT_EQ(skr_vfs_fclose(f), true);
    skr_free_vfs(abs_fs);
}

#include "llfio.hpp"

namespace llfio = LLFIO_V2_NAMESPACE;
TEST_F(FSTest, llf_asyncread)
{
}

TEST_F(FSTest, asyncread)
{
    skr_vfs_desc_t abs_fs_desc = {};
    abs_fs_desc.app_name = "fs-test";
    abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    const auto current_path = ghc::filesystem::current_path().u8string();
    abs_fs_desc.override_mount_dir = current_path.c_str();
    auto abs_fs = skr_create_vfs(&abs_fs_desc);
    EXPECT_NE(abs_fs, nullptr);
    EXPECT_NE(abs_fs->mount_dir, nullptr);
    skr::io::RAM::initialize();
    uint8_t bytes[1024];
    skr_async_io_request_t request;
    skr::io::RAM::request(abs_fs, "testfile", bytes, 0, 1024, &request);
    std::cout << (const char*)bytes << std::endl;
    // while (!request.is_ready()) {}
    std::cout << (const char*)bytes << std::endl;
    skr_free_vfs(abs_fs);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}