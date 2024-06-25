#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/type_registry.hpp"
#include "SkrTestFramework/framework.hpp"

inline static void _EXPECT_OK(sugoi::TypeRegisterResult&& r)
{
    using namespace skr::archive;
    r.and_then([](auto type_index) {
        EXPECT_EQ(true, true);
    })
    .error_then([](auto error) {
        EXPECT_EQ(true, false);
    });
}

inline static void _EXPECT_ERROR(sugoi::TypeRegisterResult&& r, sugoi::TypeRegisterError err_code)
{
    using namespace skr::archive;
    r.and_then([](auto type_index) {
        EXPECT_EQ(true, false);
    })
    .error_then([=](auto error) {
        EXPECT_EQ(error, err_code);
    });
}

// for better ide display
#define EXPECT_OK _EXPECT_OK
#define EXPECT_ERROR _EXPECT_ERROR

