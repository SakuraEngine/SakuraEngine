#include "gtest/gtest.h"
#include "misc/make_zeroed.hpp"
#include "misc/log.h"
#include "async/service_thread.hpp"

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
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}