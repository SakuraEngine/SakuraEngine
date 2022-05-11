#include "gtest/gtest.h"
#include "platform/vfs.h"
#include <string>
#include <iostream>
#include <ghc/filesystem.hpp>
#include "utils/io.hpp"
#include "utils/log.h"

class FSTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        skr_vfs_desc_t abs_fs_desc = {};
        abs_fs_desc.app_name = "fs-test";
        abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
        abs_fs = skr_create_vfs(&abs_fs_desc);
        EXPECT_NE(abs_fs, nullptr);
        const auto current_path = ghc::filesystem::current_path().u8string();
        EXPECT_EQ(std::string(abs_fs->mount_dir), current_path);
    }
    void TearDown() override
    {
        skr_free_vfs(abs_fs);
    }
    skr_vfs_t* abs_fs = nullptr;
};

TEST_F(FSTest, mount)
{
    skr_vfs_desc_t fs_desc = {};
    fs_desc.app_name = "fs-test";
    fs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto fs = skr_create_vfs(&fs_desc);
    EXPECT_NE(fs, nullptr);
    EXPECT_NE(fs->mount_dir, nullptr);
    skr_free_vfs(fs);
}

TEST_F(FSTest, readwrite)
{
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
}

TEST_F(FSTest, readwrite2)
{
    auto f = skr_vfs_fopen(abs_fs, "testfile2",
    SKR_FM_READ_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
    const char8_t* string = u8"Hello, World2!";
    skr_vfs_fwrite(f, string, 0, strlen(string));
    char8_t string_out[256];
    std::memset((void*)string_out, 0, 256);
    skr_vfs_fread(f, string_out, 0, strlen(string));
    EXPECT_EQ(std::string(string_out), std::string(u8"Hello, World2!"));
    EXPECT_EQ(skr_vfs_fsize(f), strlen(string));
    EXPECT_EQ(skr_vfs_fclose(f), true);
}

TEST_F(FSTest, asyncread)
{
    skr_ram_io_service_desc_t ioServiceDesc = {};
    ioServiceDesc.name = "Test";
    auto ioService = skr::io::RAMService::create(&ioServiceDesc);
    uint8_t bytes[1024];
    memset(bytes, 0, 1024);
    skr_ram_io_t ramIO = {};
    ramIO.bytes = bytes;
    ramIO.offset = 0;
    ramIO.size = 1024;
    ramIO.path = "testfile";
    skr_async_io_request_t request;
    ioService->request(abs_fs, &ramIO, &request);
    std::cout << (const char*)bytes << std::endl;
    // while (!request.is_ready()) {}
    ioService->drain();
    std::cout << (const char*)bytes << std::endl;
    skr::io::RAMService::destroy(ioService);
}

TEST_F(FSTest, cancel)
{
    skr_ram_io_service_desc_t ioServiceDesc = {};
    ioServiceDesc.name = "Test";
    ioServiceDesc.sleep_time = SKR_ASYNC_IO_SERVICE_SLEEP_TIME_NEVER /*ms*/;
    auto ioService = skr::io::RAMService::create(&ioServiceDesc);
    uint8_t bytes[1024];
    uint8_t bytes2[1024];
    memset(bytes, 0, 1024);
    memset(bytes2, 0, 1024);
    skr_ram_io_t ramIO = {};
    ramIO.bytes = bytes;
    ramIO.offset = 0;
    ramIO.size = 1024;
    ramIO.path = "testfile2";
    skr_ram_io_t anotherRamIO = {};
    anotherRamIO.bytes = bytes2;
    anotherRamIO.offset = 0;
    anotherRamIO.size = 1024;
    anotherRamIO.path = "testfile";
    skr_async_io_request_t request;
    ioService->request(abs_fs, &ramIO, &request);
    skr_async_io_request_t anotherRequest;
    ioService->request(abs_fs, &anotherRamIO, &anotherRequest);
    // try cancel io of testfile
    bool cancelled = ioService->try_cancel(&anotherRequest);
    ioService->drain();
    if (cancelled)
    {
        SKR_LOG_INFO("request to testfile cancelled!");
        uint8_t bytes0[1024];
        memset(bytes0, 0, 1024);
        EXPECT_EQ(memcmp(bytes2, bytes0, 1024), 0);
    }
    else
    {
        SKR_LOG_INFO("request to testfile not cancelled!");
        EXPECT_EQ(std::string((const char8_t*)bytes2), std::string(u8"Hello, World!"));
    }
    EXPECT_EQ(std::string((const char8_t*)bytes), std::string(u8"Hello, World2!"));
    skr::io::RAMService::destroy(ioService);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}