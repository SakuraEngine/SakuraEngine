#include "SkrCore/platform/vfs.h"
#include "SkrGraphics/dstorage.h"
#include <SkrOS/filesystem.hpp>
#include "SkrCore/log.h"

#include <string>

#include "SkrProfile/profile.h"

#include "SkrTestFramework/framework.hpp"

static struct ProcInitializer
{
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        // ::skr_initialize_crash_handler();
        // ::skr_log_initialize_async_worker();
    }
} init;

struct VFSTest
{
    VFSTest()
    {
        idx += 1;
        skr_vfs_desc_t abs_fs_desc = {};
        abs_fs_desc.app_name = u8"fs-test";
        abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
        abs_fs = skr_create_vfs(&abs_fs_desc);
        REQUIRE(abs_fs != nullptr);

        std::error_code ec = {};
        const auto current_path = skr::filesystem::current_path(ec).string();
        REQUIRE(std::string((const char*)abs_fs->mount_dir) == current_path);
        SKR_TEST_INFO(u8"Current path: {}", (const char8_t*)current_path.c_str());
    }

    ~VFSTest()
    {
        skr_free_vfs(abs_fs);
    }

    skr_vfs_t* abs_fs = nullptr;
    static uint32_t idx;
};
uint32_t VFSTest::idx = 0;

TEST_CASE_METHOD(VFSTest, "VFSTest")
{
SUBCASE("mount")
{
    SkrZoneScopedN("mount");

    skr_vfs_desc_t fs_desc = {};
    fs_desc.app_name = u8"fs-test";
    fs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto fs = skr_create_vfs(&fs_desc);
    
    REQUIRE(fs != nullptr);
    REQUIRE(fs->mount_dir != nullptr);
    skr_free_vfs(fs);
}

SUBCASE("readwrite")
{
    SkrZoneScopedN("readwrite");

    auto f = skr_vfs_fopen(abs_fs, u8"testfile", SKR_FM_READ_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
    const char8_t* string = u8"Hello, World!";
    skr_vfs_fwrite(f, string, 0, strlen((const char*)string) + 1);
    char8_t string_out[256];
    std::memset((void*)string_out, 0, 256);
    skr_vfs_fread(f, string_out, 0, strlen((const char*)string) + 1);
    EXPECT_EQ(std::string((const char*)string_out), std::string("Hello, World!"));
    EXPECT_EQ(skr_vfs_fsize(f), strlen((const char*)string) + 1);
    EXPECT_EQ(skr_vfs_fclose(f), true);
}

SUBCASE("readwrite2")
{
    SkrZoneScopedN("readwrite2");

    auto f = skr_vfs_fopen(abs_fs, u8"testfile2", SKR_FM_READ_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
    const char8_t* string = u8"Hello, World2!";
    skr_vfs_fwrite(f, string, 0, strlen((const char*)string) + 1);
    char8_t string_out[256];
    std::memset((void*)string_out, 0, 256);
    skr_vfs_fread(f, string_out, 0, strlen((const char*)string) + 1);
    EXPECT_EQ(std::string((const char*)string_out), std::string("Hello, World2!"));
    EXPECT_EQ(skr_vfs_fsize(f), strlen((const char*)string) + 1);
    EXPECT_EQ(skr_vfs_fclose(f), true);
}

SUBCASE("seqread")
{
    SkrZoneScopedN("seqread");
    
    auto f = skr_vfs_fopen(abs_fs, u8"testfile2", SKR_FM_READ_WRITE, SKR_FILE_CREATION_OPEN_EXISTING);
    const char8_t* string = u8"Hello, World2!";
    char8_t string_out[256];
    char8_t string_out2[256];
    std::memset((void*)string_out, 0, 256);
    std::memset((void*)string_out2, 0, 256);
    skr_vfs_fread(f, string_out, 0, 2);
    skr_vfs_fread(f, string_out2, 2, 3);
    EXPECT_EQ(std::string((const char*)string_out), std::string("He"));
    EXPECT_EQ(std::string((const char*)string_out2), std::string("llo"));
    EXPECT_EQ(skr_vfs_fsize(f), strlen((const char*)string) + 1);
    EXPECT_EQ(skr_vfs_fclose(f), true);
}
}