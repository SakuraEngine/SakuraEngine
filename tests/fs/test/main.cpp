#include "SkrRT/platform/vfs.h"
#include "SkrRT/platform/crash.h"
#include "SkrRT/platform/thread.h"
#include "SkrRT/platform/dstorage.h"
#include <SkrRT/platform/filesystem.hpp>
#include "SkrRT/misc/log.h"
#include "SkrRT/misc/log.hpp"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/async/thread_job.hpp"
#include "SkrRT/async/wait_timeout.hpp"
#include "SkrRT/io/ram_io.hpp"

#include <string>
#include <iostream>

#include "tracy/Tracy.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#define EXPECT_EQ(a, b) REQUIRE((a) == (b))

static struct ProcInitializer
{
    ProcInitializer()
    {
        skr_initialize_crash_handler();
        skr_log_initialize_async_worker();

        SkrDStorageConfig config = {};
        skr_create_dstorage_instance(&config);
    }
    ~ProcInitializer()
    {
        auto inst = skr_get_dstorage_instnace();
        skr_free_dstorage_instance(inst);

        skr_log_finalize_async_worker();
        skr_finalize_crash_handler();
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

        SKR_LOG_FMT_INFO(u8"Current path: {}", (const char8_t*)current_path.c_str());
    }

    ~VFSTest()
    {
        skr_free_vfs(abs_fs);
    }

    skr_vfs_t* abs_fs = nullptr;
    static uint32_t idx;
};
uint32_t VFSTest::idx = 0;

TEST_CASE_METHOD(VFSTest, "mount")
{
    ZoneScopedN("mount");

    skr_vfs_desc_t fs_desc = {};
    fs_desc.app_name = u8"fs-test";
    fs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto fs = skr_create_vfs(&fs_desc);
    REQUIRE(fs != nullptr);
    REQUIRE(fs->mount_dir != nullptr);
    skr_free_vfs(fs);
}

TEST_CASE_METHOD(VFSTest, "readwrite")
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

TEST_CASE_METHOD(VFSTest, "readwrite2")
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

TEST_CASE_METHOD(VFSTest, "seqread")
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

TEST_CASE_METHOD(VFSTest, "asyncread")
{
    ZoneScopedN("asyncread");
    
    auto dstorage = GENERATE(as<bool>{}, true, false);
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
                SKR_LOG_INFO("async read of file %s ok", pRamIO->get_path());
            }, rq.get());
        blob = ioService->request(rq, &future);
    }

    wait_timeout([&future]()->bool
    {
        return future.is_ready();
    });
    
    // ioService->drain();
    std::cout << (const char*)blob->get_data() << std::endl;
    skr_io_ram_service_t::destroy(ioService);
    std::cout << "..." << std::endl;
}

TEST_CASE_METHOD(VFSTest, "asyncread2")
{
    ZoneScopedN("asyncread2");

    auto dstorage = GENERATE(as<bool>{}, true, false);
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
                SKR_LOG_INFO("async read of file %s ok", pRamIO->get_path());
            }, rq.get());
        blob = ioService->request(rq, &future);
    }

    wait_timeout([&future]()->bool
    {
        return future.is_ready();
    });
    
    ioService->drain();
    std::cout << (const char*)blob->get_data() << std::endl;
    skr_io_ram_service_t::destroy(ioService);
    std::cout << "..." << std::endl;

    SkrDelete(io_job_queue);
}

TEST_CASE_METHOD(VFSTest, "chunking")
{
    ZoneScopedN("chunking");

    auto dstorage = GENERATE(as<bool>{}, true, false);
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
                SKR_LOG_INFO("async read of file %s ok", pRamIO->get_path());
            }, rq.get());
        blob = ioService->request(rq, &future);
    }

    wait_timeout([&future]()->bool
    {
        return future.is_ready();
    });
    
    ioService->drain();
    std::cout << (const char*)blob->get_data() << std::endl;
    skr_io_ram_service_t::destroy(ioService);
    std::cout << "..." << std::endl;
}

#define TEST_CYCLES_COUNT 100

TEST_CASE_METHOD(VFSTest, "defer_cancel")
{
    ZoneScopedN("defer_cancel");

    auto dstorage = GENERATE(as<bool>{}, true, false);
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
            EXPECT_EQ(std::string((const char*)blob2->get_data(), blob2->get_size()), std::string("Hello, World!"));
        }
        SKR_ASSERT(std::string((const char*)blob->get_data(), blob->get_size()) == std::string("Hello, World2!"));
        
        blob.reset();
        blob2.reset();

        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("defer_cancel tested for %d times, sucess %d", TEST_CYCLES_COUNT, sucess);
}

TEST_CASE_METHOD(VFSTest, "cancel")
{
    ZoneScopedN("cancel");

    auto dstorage = GENERATE(as<bool>{}, true, false);
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
        // while (!request.is_ready()) {}
        // while (!cancelled && !future2.is_ready()) {}
        ioService->drain();

        if (future2.is_cancelled())
        {
            EXPECT_EQ(blob2->get_data(), nullptr);
            sucess++;
        }
        
        EXPECT_EQ(std::string((const char*)blob->get_data(), blob->get_size()), std::string("Hello, World2!"));
        
        blob.reset();
        blob2.reset();
        
        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("cancel tested for %d times, sucess %d", TEST_CYCLES_COUNT, sucess);
}

// this test dont works with batch NVMe queue APIs, like windows DirectStorage.
TEST_CASE_METHOD(VFSTest, "sort")
{
    auto dstorage = GENERATE(as<bool>{}, true, false);
    if (dstorage) 
        return;

    ZoneScopedN("sort");
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
        EXPECT_EQ(std::string((const char*)blob2->get_data(), blob2->get_size()), std::string("Hello, World!"));
        
        blob.reset();
        blob2.reset();
        
        skr_io_ram_service_t::destroy(ioService);
    }
    SKR_LOG_INFO("sorts tested for %d times", TEST_CYCLES_COUNT);
}