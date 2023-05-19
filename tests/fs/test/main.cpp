#include "gtest/gtest.h"
#include "platform/vfs.h"
#include "platform/thread.h"
#include <string>
#include <iostream>
#include <platform/filesystem.hpp>
#include "misc/io.h"
#include "misc/log.h"

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
    uint32_t seconds = 0;
    while (!f())
    {
        skr_thread_sleep(1);
        seconds++;
        if (seconds > seconds_timeout * 1000)
        {
            SKR_LOG_ERROR("drain timeout, force quit");
            EXPECT_TRUE(0);
            break;
        }
    }
}

TEST_F(FSTest, asyncread)
{
    skr_ram_io_service_desc_t ioServiceDesc = {};
    ioServiceDesc.name = u8"Test";
    auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
    uint8_t bytes[1024];
    memset(bytes, 0, 1024);
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    const char8_t* testfile = u8"testfile";
    ramIO.path = testfile;
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* arg){
        skr_ram_io_t* pRamIO = (skr_ram_io_t*)arg;
        SKR_LOG_INFO("async read of file %s ok", pRamIO->path);
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = &ramIO;
    skr_async_request_t request = {};
    skr_async_ram_destination_t destination = {};
    ioService->request(abs_fs, &ramIO, &request, &destination);

    wait_timeout([&request]()->bool
    {
        return request.is_ready();
    });
    
    // ioService->drain();
    std::cout << (const char*)destination.bytes << std::endl;
    skr_io_ram_service_t::destroy(ioService);
    std::cout << "..." << std::endl;
}

/*
TEST_F(FSTest, cancel)
{
    uint32_t sucess = 0;
    for (uint32_t i = 0; i < 100; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
        ioServiceDesc.lockless = false;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        skr_ram_io_t ramIO = {};
        ramIO.offset = 0;
        ramIO.path = u8"testfile2";
        skr_ram_io_t anotherRamIO = {};
        anotherRamIO.offset = 0;
        anotherRamIO.path = u8"testfile";
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

            wait_timeout([&anotherRequest]()->bool
            {
                return anotherRequest.is_ready();
            });

            EXPECT_EQ(std::string((const char*)anotherDestination.bytes, anotherDestination.size), std::string("Hello, World!"));
        }
        // while (!request.is_ready()) {}
        // while (!cancelled && !anotherRequest.is_ready()) {}
        ioService->drain();
        EXPECT_EQ(std::string((const char*)destination.bytes, destination.size), std::string("Hello, World2!"));
        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("cancel tested for %d times, sucess %d", 100, sucess);
}


TEST_F(FSTest, defer_cancel)
{
    uint32_t sucess = 0;
    for (uint32_t i = 0; i < 100; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.lockless = true;
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        skr_ram_io_t ramIO = {};
        ramIO.offset = 0;
        ramIO.path = u8"testfile2";
        skr_ram_io_t anotherRamIO = {};
        anotherRamIO.offset = 0;
        anotherRamIO.path = u8"testfile";
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

            wait_timeout([&anotherRequest]()->bool
            {
                return anotherRequest.is_ready();
            });

            EXPECT_EQ(std::string((const char*)anotherDestination.bytes, anotherDestination.size), std::string("Hello, World!"));
        }
        EXPECT_EQ(std::string((const char*)destination.bytes, destination.size), std::string("Hello, World2!"));
        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("defer_cancel tested for %d times, sucess %d", 100, sucess);
}

TEST_F(FSTest, sort)
{
    for (uint32_t i = 0; i < 100; i++)
    {
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
        ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        ioService->stop(true);
        skr_ram_io_t ramIO = {};
        skr_async_request_t request;
        ramIO.offset = 0;
        ramIO.priority = ::SKR_ASYNC_SERVICE_PRIORITY_NORMAL;
        ramIO.path = u8"testfile2";
        skr_ram_io_t anotherRamIO = {};
        skr_async_request_t anotherRequest;
        anotherRamIO.offset = 0;
        anotherRamIO.priority = ::SKR_ASYNC_SERVICE_PRIORITY_URGENT;
        anotherRamIO.path = u8"testfile";
        anotherRamIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = &request;
        anotherRamIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = 
        +[](skr_async_request_t* r, void* data) {
            skr_async_request_t& request = *(skr_async_request_t*)data;
            EXPECT_TRUE(!request.is_ready());
        };
        skr_async_ram_destination_t destination = {};
        ioService->request(abs_fs, &ramIO, &request, &destination);
        skr_async_ram_destination_t anotherDestination = {};
        ioService->request(abs_fs, &anotherRamIO, &anotherRequest, &anotherDestination);
        ioService->run();
        wait_timeout([&anotherRequest]()->bool
        {
            return anotherRequest.is_ready();
        });
        // while (!cancelled && !anotherRequest.is_ready()) {}
        ioService->drain();
        EXPECT_EQ(std::string((const char*)anotherDestination.bytes, anotherDestination.size), std::string("Hello, World!"));
        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("sorts tested for %d times", 100);
}
*/

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}