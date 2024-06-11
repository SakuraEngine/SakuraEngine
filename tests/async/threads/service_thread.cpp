#include "SkrCore/crash.h"
#include "SkrCore/log.h"
#include "SkrCore/async/async_service.h"
#include "SkrCore/async/wait_timeout.hpp"

#include "SkrTestFramework/framework.hpp"

static struct ProcInitializer {
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        // ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        // ::skr_finalize_crash_handler();
    }
} init;

class ServiceThreadTests
{
protected:
    ServiceThreadTests()
    {
    }
    ~ServiceThreadTests()
    {
    }
};

TEST_CASE_METHOD(ServiceThreadTests, "AsyncPrint")
{
    struct TestServiceThread : public skr::ServiceThread {
        TestServiceThread()
            : ServiceThread({ u8"TestService" })
        {
        }
        skr::AsyncResult serve() SKR_NOEXCEPT
        {
            if (times <= 5)
                SKR_LOG_DEBUG(u8"Hello World! %d", times++);
            else
                this->request_stop();
            return skr::ASYNC_RESULT_OK;
        }
        int32_t times = 0;
    };
    auto srv = TestServiceThread();
    SKR_LOG_DEBUG(u8"Request Run");
    srv.run();
    SKR_LOG_DEBUG(u8"Wait Stop");
    wait_timeout([&] { return srv.get_status() == skr::ServiceThread::kStatusStopped; });
    SKR_LOG_DEBUG(u8"Stopped");
    EXPECT_EQ(srv.times, 6);
    SKR_LOG_DEBUG(u8"Wait Exit");
    srv.exit();
    SKR_LOG_DEBUG(u8"Exitted");
}

TEST_CASE_METHOD(ServiceThreadTests, "AsyncPrint2")
{
    for (uint32_t i = 0; i < 10; i++)
    {
        struct TestServiceThread : public skr::ServiceThread {
            TestServiceThread()
                : ServiceThread({ u8"TestService" })
            {
            }
            skr::AsyncResult serve() SKR_NOEXCEPT
            {
                if (
                ((times <= 5) && (times >= 0)) ||
                ((times <= 20) && (times >= 15)))
                {
                    SKR_LOG_DEBUG(u8"Hello World! %d", times++);
                }
                else
                {
                    this->request_stop();
                }
                return skr::ASYNC_RESULT_OK;
            }
            int32_t times = 0;
        };
        auto srv = TestServiceThread();
        srv.run();
        wait_timeout([&] { return srv.get_status() == skr::ServiceThread::kStatusStopped; });
        EXPECT_EQ(srv.times, 6);

        srv.times = 15;
        srv.run();
        wait_timeout([&] { return srv.get_status() == skr::ServiceThread::kStatusStopped; });
        EXPECT_EQ(srv.times, 21);

        srv.exit();
    }
}