#include "SkrRT/platform/crash.h"
#include "SkrRT/misc/log.h"
#include <catch2/catch_test_macros.hpp>

#ifndef EXPECT_EQ
#define EXPECT_EQ(a, b) REQUIRE(a == b)
#endif

static struct ProcInitializer
{
    ProcInitializer()
    {
        ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        ::skr_finalize_crash_handler();
    }
} init;

#include "binary.cpp"
#include "json.cpp"
#include "bitpack.cpp"