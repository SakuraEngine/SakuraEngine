#include "gtest/gtest.h"
#include "misc/log.h"

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    log_finalize();
    return result;
}