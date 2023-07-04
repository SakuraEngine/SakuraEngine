#include "gtest/gtest.h"
#include "misc/log.h"

int main(int argc, char** argv)
{
    log_initialize_async_worker();
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    log_finalize();
    return result;
}