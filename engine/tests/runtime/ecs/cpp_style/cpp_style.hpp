#pragma once
#include "SkrRuntime/sugoi/sugoi.h"
#include "SkrRuntime/sugoi/array.hpp"
#include "SkrRuntime/sugoi/type_registry.hpp"
#include "SkrTestFramework/framework.hpp"
#include "cpp_style.generated.h"

template <typename Result>
inline static void _EXPECT_OK(Result&& r)
{
    r.and_then([](auto type_index) {
         EXPECT_EQ(true, true);
     })
        .error_then([](auto error) {
            EXPECT_EQ(true, false);
        });
}

template <typename Result, typename ErrorCode>
inline static void _EXPECT_ERROR(Result&& r, ErrorCode err_code)
{
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

struct [[sattr(
    guid = "12d2301d-b741-4191-acf6-d4fa7c7f64d5"
    serde = @enable
    ecs.comp = @enable
)]] IntComponent
{
    int v;
};

struct [[sattr(
    guid = "9bcb423f-aa5b-45dc-87b8-0f805686e777"
    serde = @enable
    ecs.comp = @enable
)]] FloatComponent
{
    float v;
};

struct [[sattr(
    guid = "a54016fa-a6b7-4224-87a7-cb1da8abe8ed"
    serde = @enable
    ecs.comp = @enable
)]] SharedComponent
{
    int i;
    float f;
};