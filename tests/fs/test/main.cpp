#include "gtest/gtest.h"
#include "platform/vfs.h"
#include "platform/thread.h"
#include <string>
#include <iostream>
#include <platform/filesystem.hpp>
#include "io/io.h"
#include "misc/log.h"

#include "tracy/Tracy.hpp"

class FSTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        skr_vfs_desc_t abs_fs_desc = {};
        abs_fs_desc.app_name = u8"fs-test";
        abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
        abs_fs = skr_create_vfs(&abs_fs_desc);
        EXPECT_NE(abs_fs, nullptr);
        std::error_code ec = {};
        const auto current_path = skr::filesystem::current_path(ec).string();
        EXPECT_EQ(std::string((const char*)abs_fs->mount_dir), current_path);
    }

    void TearDown() override
    {
        skr_free_vfs(abs_fs);
    }

    skr_vfs_t* abs_fs = nullptr;
};

TEST_F(FSTest, mount)
{
    ZoneScopedN("mount");

    skr_vfs_desc_t fs_desc = {};
    fs_desc.app_name = u8"fs-test";
    fs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto fs = skr_create_vfs(&fs_desc);
    EXPECT_NE(fs, nullptr);
    EXPECT_NE(fs->mount_dir, nullptr);
    skr_free_vfs(fs);
}

TEST_F(FSTest, readwrite)
{
    ZoneScopedN("readwrite");

    auto f = skr_vfs_fopen(abs_fs, u8"testfile", SKR_FM_READ_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
    const char8_t* string = u8"Hello, World!";
    skr_vfs_fwrite(f, string, 0, strlen((const char*)string));
    char8_t string_out[256];
    std::memset((void*)string_out, 0, 256);
    skr_vfs_fread(f, string_out, 0, strlen((const char*)string));
    EXPECT_EQ(std::string((const char*)string_out), std::string("Hello, World!"));
    EXPECT_EQ(skr_vfs_fsize(f), strlen((const char*)string));
    EXPECT_EQ(skr_vfs_fclose(f), true);
}

TEST_F(FSTest, readwrite2)
{
    ZoneScopedN("readwrite2");

    auto f = skr_vfs_fopen(abs_fs, u8"testfile2", SKR_FM_READ_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
    const char8_t* string = u8"Hello, World2!";
    skr_vfs_fwrite(f, string, 0, strlen((const char*)string));
    char8_t string_out[256];
    std::memset((void*)string_out, 0, 256);
    skr_vfs_fread(f, string_out, 0, strlen((const char*)string));
    EXPECT_EQ(std::string((const char*)string_out), std::string("Hello, World2!"));
    EXPECT_EQ(skr_vfs_fsize(f), strlen((const char*)string));
    EXPECT_EQ(skr_vfs_fclose(f), true);
}

TEST_F(FSTest, seqread)
{
    ZoneScopedN("seqread");
    
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
    EXPECT_EQ(skr_vfs_fsize(f), strlen((const char*)string));
    EXPECT_EQ(skr_vfs_fclose(f), true);
}

template<typename F>
void wait_timeout(F f, uint32_t seconds_timeout = 3)
{
    ZoneScopedN("WaitTimeOut");

    uint32_t seconds = 0;
    while (!f())
    {
        skr_thread_sleep(1);
        seconds++;
        if (seconds > seconds_timeout * 100)
        {
            SKR_LOG_ERROR("drain timeout, force quit");
            EXPECT_TRUE(0);
            break;
        }
    }
}

TEST_F(FSTest, asyncread)
{
    ZoneScopedN("asyncread");

    skr_ram_io_service_desc_t ioServiceDesc = {};
    ioServiceDesc.name = u8"Test";
    auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
    
    uint8_t bytes[1024];
    memset(bytes, 0, 1024);

    skr_io_request_t ramIO = {};
    const char8_t* testfile = u8"testfile";
    ramIO.path = testfile;
    ramIO.block.offset = 0;
    ramIO.block.size = 0;
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_READ_OK] = 
        +[](skr_io_future_t* future, skr_io_request_t* request, void* arg){
            auto pRamIO = (skr_io_request_t*)arg;
            SKR_LOG_INFO("async read of file %s ok", pRamIO->path);
        };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_READ_OK] = &ramIO;

    skr_io_future_t future = {};
    skr_async_ram_destination_t destination = {};
    destination.bytes = bytes;
    ioService->request(abs_fs, &ramIO, &future, &destination);

    wait_timeout([&future]()->bool
    {
        return future.is_ready();
    });
    
    // ioService->drain();
    std::cout << (const char*)destination.bytes << std::endl;
    skr_io_ram_service_t::destroy(ioService);
    std::cout << "..." << std::endl;
}

#define TEST_CYCLES_COUNT 50

TEST_F(FSTest, cancel)
{
    ZoneScopedN("cancel");

    uint32_t sucess = 0;
    for (uint32_t i = 0; i < TEST_CYCLES_COUNT; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
        ioServiceDesc.lockless = false;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        ioService->set_sleep_time(0); // make test faster
        skr_io_request_t ramIO = {};
        ramIO.block.offset = 0;
        ramIO.block.size = 0;
        ramIO.path = u8"testfile2";
        skr_io_request_t anotherRamIO = {};
        anotherRamIO.block.offset = 0;
        anotherRamIO.block.size = 0;
        anotherRamIO.path = u8"testfile";
        skr_io_future_t request = {};
        skr_async_ram_destination_t destination = {};
        ioService->request(abs_fs, &ramIO, &request, &destination);
        skr_io_future_t anotherRequest = {};
        skr_async_ram_destination_t anotherDestination = {};
        ioService->request(abs_fs, &anotherRamIO, &anotherRequest, &anotherDestination);
        // try cancel io of testfile
        ioService->cancel(&anotherRequest);
        // while (!request.is_ready()) {}
        // while (!cancelled && !anotherRequest.is_ready()) {}
        ioService->drain();
        
        if (anotherRequest.is_cancelled())
        {
            EXPECT_EQ(anotherDestination.bytes, nullptr);
            sucess++;
        }
        
        EXPECT_EQ(std::string((const char*)destination.bytes, destination.size), std::string("Hello, World2!"));
        
        if (destination.bytes) sakura_free(destination.bytes);
        if (anotherDestination.bytes) sakura_free(anotherDestination.bytes);
        
        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("cancel tested for %d times, sucess %d", TEST_CYCLES_COUNT, sucess);
}

TEST_F(FSTest, defer_cancel)
{
    ZoneScopedN("defer_cancel");

    uint32_t sucess = 0;
    for (uint32_t i = 0; i < TEST_CYCLES_COUNT; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.lockless = true;
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        ioService->set_sleep_time(0); // make test faster
        skr_io_request_t ramIO = {};
        ramIO.path = u8"testfile2";
        skr_io_request_t anotherRamIO = {};
        anotherRamIO.path = u8"testfile";
        skr_io_future_t request;
        skr_async_ram_destination_t destination = {};
        ioService->request(abs_fs, &ramIO, &request, &destination);
        skr_io_future_t anotherRequest;
        skr_async_ram_destination_t anotherDestination = {};
        ioService->request(abs_fs, &anotherRamIO, &anotherRequest, &anotherDestination);
        // try cancel io of testfile
        ioService->cancel(&anotherRequest);
        ioService->drain();
        if (anotherRequest.is_cancelled())
        {
            EXPECT_EQ(anotherDestination.bytes, nullptr);
            sucess++;
        }
        else
        {
            EXPECT_TRUE(anotherRequest.is_enqueued() || anotherRequest.is_ram_loading() || anotherRequest.is_ready());

            wait_timeout([&anotherRequest]()->bool
            {
                return anotherRequest.is_ready();
            });

            EXPECT_EQ(std::string((const char*)anotherDestination.bytes, anotherDestination.size), std::string("Hello, World!"));
        }
        EXPECT_EQ(std::string((const char*)destination.bytes, destination.size), std::string("Hello, World2!"));
        
        if (destination.bytes) sakura_free(destination.bytes);
        if (anotherDestination.bytes) sakura_free(anotherDestination.bytes);

        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("defer_cancel tested for %d times, sucess %d", TEST_CYCLES_COUNT, sucess);
}

TEST_F(FSTest, sort)
{
    ZoneScopedN("sort");

    for (uint32_t i = 0; i < TEST_CYCLES_COUNT; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
        ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        ioService->set_sleep_time(0); // make test faster
        ioService->stop(true);
        skr_io_request_t ramIO = {};
        skr_io_future_t future;
        ramIO.priority = ::SKR_ASYNC_SERVICE_PRIORITY_NORMAL;
        ramIO.path = u8"testfile2";
        skr_io_request_t anotherRamIO = {};
        skr_io_future_t anotherRequest;
        anotherRamIO.priority = ::SKR_ASYNC_SERVICE_PRIORITY_URGENT;
        anotherRamIO.path = u8"testfile";
        anotherRamIO.callback_datas[SKR_ASYNC_IO_STATUS_READ_OK] = &future;
        anotherRamIO.callbacks[SKR_ASYNC_IO_STATUS_READ_OK] = 
        +[](skr_io_future_t* f, skr_io_request_t* request, void* data) {
            auto future = (skr_io_future_t*)data;
            EXPECT_TRUE(!future->is_ready());
        };
        skr_async_ram_destination_t destination = {};
        ioService->request(abs_fs, &ramIO, &future, &destination);
        skr_async_ram_destination_t anotherDestination = {};
        ioService->request(abs_fs, &anotherRamIO, &anotherRequest, &anotherDestination);
        ioService->run();
        ioService->drain();

        wait_timeout([&anotherRequest]()->bool
        {
            return anotherRequest.is_ready();
        });
        // while (!cancelled && !anotherRequest.is_ready()) {}
        EXPECT_EQ(std::string((const char*)anotherDestination.bytes, anotherDestination.size), std::string("Hello, World!"));
        
        if (destination.bytes) sakura_free(destination.bytes);
        if (anotherDestination.bytes) sakura_free(anotherDestination.bytes);
        
        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("sorts tested for %d times", TEST_CYCLES_COUNT);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}