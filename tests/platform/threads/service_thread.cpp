#include "gtest/gtest.h"
#include "platform/crash.h"
#include "misc/make_zeroed.hpp"
#include "misc/log.h"
#include "async/async_service.h"

TEST(ServiceThread, AsyncPrint)
{
    struct TestServiceThread : public skr::ServiceThread
    {
        TestServiceThread() : ServiceThread({u8"TestService"}) {}
        skr::AsyncResult serve() SKR_NOEXCEPT
        {
            if (times <= 5)
            {
                SKR_LOG_DEBUG("Hello World! %d", times++);
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
    SKR_LOG_DEBUG("Request Run");
    srv.run();
    SKR_LOG_DEBUG("Wait Stop");
    srv.wait_stop();
    SKR_LOG_DEBUG("Stopped");
    EXPECT_EQ(srv.times, 6);
    SKR_LOG_DEBUG("Wait Exit");
    srv.exit();
    SKR_LOG_DEBUG("Exitted");
}

TEST(ServiceThread, AsyncPrint2)
{
    struct TestServiceThread : public skr::ServiceThread
    {
        TestServiceThread() : ServiceThread({u8"TestService"}) {}
        skr::AsyncResult serve() SKR_NOEXCEPT
        {
            if (
                ((times <= 5) && (times >= 0)) ||
                ((times <= 20) && (times >= 15))
            )
            {
                SKR_LOG_DEBUG("Hello World! %d", times++);
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
    srv.wait_stop();
    EXPECT_EQ(srv.times, 6);

    srv.times = 15;
    srv.run();
    srv.wait_stop();
    EXPECT_EQ(srv.times, 21);
    
    srv.exit();
}

int main(int argc, char** argv)
{
    skr_initialize_crash_handler();
    skr_log_initialize_async_worker();

    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();

    skr_log_finalize_async_worker(); 
    skr_finalize_crash_handler();
    return result;
}