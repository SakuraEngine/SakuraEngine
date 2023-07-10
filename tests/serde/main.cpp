#include "gtest/gtest.h"
#include "platform/crash.h"
#include "misc/log.h"

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