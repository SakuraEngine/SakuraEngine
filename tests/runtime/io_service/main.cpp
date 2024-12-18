#include "SkrCore/platform/vfs.h"
#include "SkrCore/crash.h"
#include "SkrOS/thread.h"
#include "SkrGraphics/dstorage.h"
#include <SkrOS/filesystem.hpp>
#include "SkrCore/log.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/async/thread_job.hpp"
#include "SkrCore/async/wait_timeout.hpp"
#include "SkrRT/io/ram_io.hpp"

#include <string>

#include "SkrProfile/profile.h"

#include "SkrTestFramework/framework.hpp"

static struct ProcInitializer
{
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        // ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();

        SkrDStorageConfig config = {};
        ::skr_create_dstorage_instance(&config);
    }
    ~ProcInitializer()
    {
        auto inst = skr_get_dstorage_instnace();
        ::skr_free_dstorage_instance(inst);

        ::skr_log_finalize_async_worker();
        // ::skr_finalize_crash_handler();
    }
} init;

struct IOServiceTest
{
    IOServiceTest()
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

    ~IOServiceTest()
    {
        skr_free_vfs(abs_fs);
    }

    skr_vfs_t* abs_fs = nullptr;
    static uint32_t idx;
};
uint32_t IOServiceTest::idx = 0;

TEST_CASE_METHOD(IOServiceTest, "IOServiceTest")
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

for (uint32_t i = 0; i < 1; i++)
{
    const auto dstorage = (i == 0);
    SUBCASE("asyncread")
    {
        SkrZoneScopedN("asyncread");
        
        SKR_TEST_INFO(u8"dstorage enabled: {}", dstorage);
        
        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.use_dstorage = dstorage;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        ioService->run();

        skr_io_future_t future = {};
        skr::BlobId blob = nullptr;
        {
            auto rq = ioService->open_request();
            rq->set_vfs(abs_fs);
            rq->set_path(u8"testfile2");
            rq->add_block({}); // read all
            rq->add_callback(SKR_IO_STAGE_COMPLETED,
                +[](skr_io_future_t* future, skr_io_request_t* request, void* arg){
                    auto pRamIO = (skr_io_request_t*)arg;
                    SKR_TEST_INFO(u8"async read of file {} ok", pRamIO->get_path());
                }, rq.get());
            blob = ioService->request(rq, &future);
        }

        wait_timeout([&future]()->bool
        {
            return future.is_ready();
        });
        
        // ioService->drain();
        EXPECT_EQ(std::string((const char*)blob->get_data()), std::string("Hello, World2!"));
        skr_io_ram_service_t::destroy(ioService);
    }

    SUBCASE("asyncread2")
    {
        SkrZoneScopedN("asyncread2");

        SKR_TEST_INFO(u8"dstorage enabled: {}", dstorage);

        auto jqDesc = make_zeroed<skr::JobQueueDesc>();
        jqDesc.thread_count = 1;
        jqDesc.priority = SKR_THREAD_ABOVE_NORMAL;
        jqDesc.name = u8"Tool-IOJobQueue";
        auto io_job_queue = SkrNew<skr::JobQueue>(jqDesc);

        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.use_dstorage = dstorage;
        ioServiceDesc.io_job_queue = io_job_queue;
        ioServiceDesc.callback_job_queue = io_job_queue;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        ioService->run();

        skr_io_future_t future = {};
        skr::BlobId blob = nullptr;
        {
            auto rq = ioService->open_request();
            rq->set_vfs(abs_fs);
            rq->set_path(u8"testfile2");
            rq->add_block({}); // read all
            rq->add_callback(SKR_IO_STAGE_COMPLETED,
                +[](skr_io_future_t* future, skr_io_request_t* request, void* arg){
                    auto pRamIO = (skr_io_request_t*)arg;
                    SKR_TEST_INFO(u8"async read of file {} ok", pRamIO->get_path());
                }, rq.get());
            blob = ioService->request(rq, &future);
        }

        wait_timeout([&future]()->bool
        {
            return future.is_ready();
        });
        
        ioService->drain();
        EXPECT_EQ(std::string((const char*)blob->get_data()), std::string("Hello, World2!"));
        skr_io_ram_service_t::destroy(ioService);

        SkrDelete(io_job_queue);
    }

    SUBCASE("chunking")
    {
        SkrZoneScopedN("chunking");

        SKR_TEST_INFO(u8"dstorage enabled: {}", dstorage);

        skr_ram_io_service_desc_t ioServiceDesc = {};
        ioServiceDesc.name = u8"Test";
        ioServiceDesc.use_dstorage = dstorage;
        auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
        ioService->run();

        skr_io_future_t future = {};
        skr::BlobId blob = nullptr;
        {
            auto rq = ioService->open_request();
            rq->set_vfs(abs_fs);
            rq->set_path(u8"testfile2");
            rq->add_block({}); // read all
            rq->add_callback(SKR_IO_STAGE_COMPLETED,
                +[](skr_io_future_t* future, skr_io_request_t* request, void* arg){
                    auto pRamIO = (skr_io_request_t*)arg;
                    SKR_TEST_INFO(u8"async read of file {} ok", pRamIO->get_path());
                }, rq.get());
            blob = ioService->request(rq, &future);
        }

        wait_timeout([&future]()->bool
        {
            return future.is_ready();
        });
        
        ioService->drain();
        EXPECT_EQ(std::string((const char*)blob->get_data()), std::string("Hello, World2!"));
        skr_io_ram_service_t::destroy(ioService);
    }

    #define TEST_CYCLES_COUNT 100

    SUBCASE("cancel")
    {
        SkrZoneScopedN("cancel");

        SKR_TEST_INFO(u8"dstorage enabled: {}", dstorage);

        uint32_t sucess = 0;
        for (uint32_t i = 0; i < TEST_CYCLES_COUNT; i++)
        {
            skr_ram_io_service_desc_t ioServiceDesc = {};
            ioServiceDesc.name = u8"Test";
            ioServiceDesc.use_dstorage = dstorage;
            ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
            auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
            ioService->set_sleep_time(0); // make test faster
            ioService->run();

            skr_io_future_t future = {};
            skr::BlobId blob = nullptr;
            skr_io_future_t future2 = {};
            skr::BlobId blob2 = nullptr;
            {
                auto rq = ioService->open_request();
                rq->set_vfs(abs_fs);
                rq->set_path(u8"testfile2");
                rq->add_block({}); // read all
                blob = ioService->request(rq, &future);
            }
            {
                auto rq2 = ioService->open_request();
                rq2->set_vfs(abs_fs);
                rq2->set_path(u8"testfile");
                rq2->add_block({}); // read all
                blob2 = ioService->request(rq2, &future2);
            }
            // try cancel io of testfile
            ioService->cancel(&future2);
            ioService->drain();
            if (future2.is_cancelled())
            {
                EXPECT_EQ(blob2->get_data(), nullptr);
                sucess++;
            }
            else if (future2.is_ready())
            {
                EXPECT_EQ(std::string((const char*)blob2->get_data()), std::string("Hello, World!"));
            }
            EXPECT_EQ(std::string((const char*)blob->get_data()), std::string("Hello, World2!"));
            
            blob.reset();
            blob2.reset();
            
            skr_io_ram_service_t::destroy(ioService);
        }
        SKR_TEST_INFO(u8"cancel tested for {} times, sucess {}", TEST_CYCLES_COUNT, sucess);
    }

    // this test dont works with batch NVMe queue APIs, like windows DirectStorage.
    SUBCASE("sort")
    {
        SKR_TEST_INFO(u8"dstorage enabled: {}", dstorage);

        if (dstorage) 
            return;

        SkrZoneScopedN("sort");
        for (uint32_t i = 0; i < TEST_CYCLES_COUNT; i++)
        {
            skr_ram_io_service_desc_t ioServiceDesc = {};
            ioServiceDesc.name = u8"Test";
            ioServiceDesc.use_dstorage = dstorage;
            ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
            auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
            ioService->set_sleep_time(0); // make test faster

            skr_io_future_t future = {};
            skr::BlobId blob = nullptr;
            skr_io_future_t future2 = {};
            skr::BlobId blob2 = nullptr;
            {
                auto rq = ioService->open_request();
                rq->set_vfs(abs_fs);
                rq->set_path(u8"testfile");
                rq->add_block({}); // read all
                rq->add_callback(SKR_IO_STAGE_COMPLETED, 
                +[](skr_io_future_t* f, skr_io_request_t* request, void* data) {
                    auto future2 = (skr_io_future_t*)data;
                    REQUIRE(future2->is_ready());
                }, &future2);
                blob = ioService->request(rq, &future, SKR_ASYNC_SERVICE_PRIORITY_NORMAL);
            }
            {
                auto rq2 = ioService->open_request();
                rq2->set_vfs(abs_fs);
                rq2->set_path(u8"testfile");
                rq2->add_block({}); // read all
                rq2->add_callback(SKR_IO_STAGE_COMPLETED, 
                +[](skr_io_future_t* f, skr_io_request_t* request, void* data) {
                    auto future = (skr_io_future_t*)data;
                    REQUIRE(!future->is_ready());
                }, &future);
                blob2 = ioService->request(rq2, &future2, SKR_ASYNC_SERVICE_PRIORITY_URGENT);
            }
            ioService->run();
            ioService->drain();

            wait_timeout([&future2]()->bool
            {
                return future2.is_ready();
            });
            // while (!cancelled && !future2.is_ready()) {}
            EXPECT_EQ(std::string((const char*)blob2->get_data()), std::string("Hello, World!"));
            
            blob.reset();
            blob2.reset();
            
            skr_io_ram_service_t::destroy(ioService);
        }
        SKR_TEST_INFO(u8"sorts tested for {} times", TEST_CYCLES_COUNT);
    }
}
}