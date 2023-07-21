#pragma once
#include "SkrRT/platform/guid.hpp" // IWYU pragma: export
#include "SkrRT/platform/crash.h" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export
#include "SkrRT/containers/sptr.hpp" // IWYU pragma: export

#include <catch2/catch_test_macros.hpp>

#define EXPECT_EQ(a, b) REQUIRE(a == b)
#define EXPECT_NE(a, b) REQUIRE(a != b)
#define EXPECT_FALSE(v) REQUIRE(!(v))

struct SPTRTestsBase
{
    static struct ProcInitializer
    {
        ProcInitializer();
        ~ProcInitializer();
    } init;
};