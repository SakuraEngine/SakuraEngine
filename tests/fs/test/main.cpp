#include "gtest/gtest.h"
#include "platform/vfs.h"
#include <string>
#include <iostream>
#include <platform/filesystem.hpp>
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
        std::error_code ec = {};
        const auto current_path = skr::filesystem::current_path(ec).u8string();
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

TEST_F(FSTest, seqread)
{
    auto f = skr_vfs_fopen(abs_fs, "testfile2",
    SKR_FM_READ_WRITE, SKR_FILE_CREATION_OPEN_EXISTING);
    const char8_t* string = u8"Hello, World2!";
    char8_t string_out[256];
    char8_t string_out2[256];
    std::memset((void*)string_out, 0, 256);
    std::memset((void*)string_out2, 0, 256);
    skr_vfs_fread(f, string_out, 0, 2);
    skr_vfs_fread(f, string_out2, 2, 3);
    EXPECT_EQ(std::string(string_out), std::string(u8"He"));
    EXPECT_EQ(std::string(string_out2), std::string(u8"llo"));
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
    ramIO.offset = 0;
    const char* testfile = "testfile";
    ramIO.path = testfile;
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* arg){
        skr_ram_io_t* pRamIO = (skr_ram_io_t*)arg;
        SKR_LOG_INFO("async read of file %s ok", pRamIO->path);
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = &ramIO;
    skr_async_request_t request = {};
    skr_async_ram_destination_t destination = {};
    ioService->request(abs_fs, &ramIO, &request, &destination);
    while (!request.is_ready()) {}
    // ioService->drain();
    std::cout << (const char*)destination.bytes << std::endl;
    skr::io::RAMService::destroy(ioService);
}

TEST_F(FSTest, cancel)
{
    uint32_t sucess = 0;
    for (uint32_t i = 0; i < 100; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = "Test";
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX /*ms*/;
        ioServiceDesc.lockless = false;
        auto ioService = skr::io::RAMService::create(&ioServiceDesc);
        skr_ram_io_t ramIO = {};
        ramIO.offset = 0;
        ramIO.path = "testfile2";
        skr_ram_io_t anotherRamIO = {};
        anotherRamIO.offset = 0;
        anotherRamIO.path = "testfile";
        skr_async_request_t request;
        skr_async_ram_destination_t destination = {};
        ioService->request(abs_fs, &ramIO, &request, &destination);
        skr_async_request_t anotherRequest;
        skr_async_ram_destination_t anotherDestination = {};
        ioService->request(abs_fs, &anotherRamIO, &anotherRequest, &anotherDestination);
        // try cancel io of testfile
        bool cancelled = ioService->try_cancel(&anotherRequest);
        if (cancelled)
        {
            EXPECT_EQ(anotherRequest.get_status(), SKR_ASYNC_IO_STATUS_CANCELLED);
            EXPECT_TRUE(anotherRequest.is_cancelled());
            EXPECT_EQ(anotherDestination.bytes, nullptr);
            sucess++;
        }
        else
        {
            EXPECT_TRUE(anotherRequest.is_enqueued() || anotherRequest.is_ram_loading() || anotherRequest.is_ready());
            while (!anotherRequest.is_ready()) {}
            EXPECT_EQ(std::string((const char8_t*)anotherDestination.bytes, anotherDestination.size), std::string(u8"Hello, World!"));
        }
        // while (!request.is_ready()) {}
        // while (!cancelled && !anotherRequest.is_ready()) {}
        ioService->drain();
        EXPECT_EQ(std::string((const char8_t*)destination.bytes, destination.size), std::string(u8"Hello, World2!"));
        skr::io::RAMService::destroy(ioService);
    }
    SKR_LOG_INFO("cancel tested for %d times, sucess %d", 100, sucess);
}


TEST_F(FSTest, defer_cancel)
{
    uint32_t sucess = 0;
    for (uint32_t i = 0; i < 100; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = "Test";
        ioServiceDesc.lockless = true;
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX /*ms*/;
        auto ioService = skr::io::RAMService::create(&ioServiceDesc);
        skr_ram_io_t ramIO = {};
        ramIO.offset = 0;
        ramIO.path = "testfile2";
        skr_ram_io_t anotherRamIO = {};
        anotherRamIO.offset = 0;
        anotherRamIO.path = "testfile";
        skr_async_request_t request;
        skr_async_ram_destination_t destination = {};
        ioService->request(abs_fs, &ramIO, &request, &destination);
        skr_async_request_t anotherRequest;
        skr_async_ram_destination_t anotherDestination = {};
        ioService->request(abs_fs, &anotherRamIO, &anotherRequest, &anotherDestination);
        // try cancel io of testfile
        ioService->defer_cancel(&anotherRequest);
        ioService->drain();
        if (anotherRequest.is_cancelled())
        {
            EXPECT_EQ(anotherDestination.bytes, nullptr);
            sucess++;
        }
        else
        {
            EXPECT_TRUE(anotherRequest.is_enqueued() || anotherRequest.is_ram_loading() || anotherRequest.is_ready());
            while (!anotherRequest.is_ready()) {}
            EXPECT_EQ(std::string((const char8_t*)anotherDestination.bytes, anotherDestination.size), std::string(u8"Hello, World!"));
        }
        EXPECT_EQ(std::string((const char8_t*)destination.bytes, destination.size), std::string(u8"Hello, World2!"));
        skr::io::RAMService::destroy(ioService);
    }
    SKR_LOG_INFO("defer_cancel tested for %d times, sucess %d", 100, sucess);
}

TEST_F(FSTest, sort)
{
    for (uint32_t i = 0; i < 100; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = "Test";
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX /*ms*/;
        ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
        auto ioService = skr::io::RAMService::create(&ioServiceDesc);
        ioService->stop(true);
        skr_ram_io_t ramIO = {};
        ramIO.offset = 0;
        ramIO.priority = ::SKR_ASYNC_SERVICE_PRIORITY_NORMAL;
        ramIO.path = "testfile2";
        skr_ram_io_t anotherRamIO = {};
        anotherRamIO.offset = 0;
        anotherRamIO.priority = ::SKR_ASYNC_SERVICE_PRIORITY_URGENT;
        anotherRamIO.path = "testfile";
        skr_async_request_t request;
        skr_async_ram_destination_t destination = {};
        ioService->request(abs_fs, &ramIO, &request, &destination);
        skr_async_request_t anotherRequest;
        skr_async_ram_destination_t anotherDestination;
        ioService->request(abs_fs, &anotherRamIO, &anotherRequest, &anotherDestination);
        ioService->run();
        while (!anotherRequest.is_ready())
        {
            EXPECT_TRUE(!request.is_ready());
        }
        // while (!cancelled && !anotherRequest.is_ready()) {}
        ioService->drain();
        EXPECT_EQ(std::string((const char8_t*)anotherDestination.bytes, anotherDestination.size), std::string(u8"Hello, World!"));
        skr::io::RAMService::destroy(ioService);
    }
    SKR_LOG_INFO("sorts tested for %d times", 100);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}