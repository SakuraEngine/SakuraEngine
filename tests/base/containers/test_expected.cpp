#include "container_test_types.hpp"
#include "SkrTestFramework/framework.hpp"
#include "SkrBase/types/expected.hpp"

TEST_CASE("test expected(int)")
{
    enum class ErrorCodes
    {
        ArgsError
    };
    using R = skr::Expected<ErrorCodes, int>;

    auto f0 = [](int v_0_5) -> R {
        if (v_0_5 < 0 || v_0_5 > 5)
            return ErrorCodes::ArgsError;
        return v_0_5 - 2;
    };

    auto err = f0(100);
    // operator==
    REQUIRE_EQ(err, ErrorCodes::ArgsError);
    REQUIRE_NE(err, 98);
    // error_then
    err.error_then([&](ErrorCodes e) {
        REQUIRE(err.has_error());
        REQUIRE_FALSE(err.has_value());
        REQUIRE_EQ(e, ErrorCodes::ArgsError);
    });

    auto success = f0(4);
    success.and_then([&](int v) {
        REQUIRE(success.has_value());
        REQUIRE_FALSE(success.has_error());
        REQUIRE_EQ(v, 2);
    });
    REQUIRE_EQ(f0(5).value(), 3);
    REQUIRE_EQ(f0(3).value(), 1);
    // operator==
    REQUIRE_EQ(f0(5), 3);
    REQUIRE_NE(f0(5), 1);
    REQUIRE_NE(f0(5), ErrorCodes::ArgsError);

    REQUIRE_NE(f0(3), 3);
    REQUIRE_EQ(f0(3), 1);
    REQUIRE_NE(f0(3), ErrorCodes::ArgsError);
}

TEST_CASE("test expected(void)")
{
    enum class ErrorCodes
    {
        ArgsError
    };
    using R = skr::Expected<ErrorCodes, void>;

    auto f0 = [](int v_0_5) -> R {
        if (v_0_5 < 0 || v_0_5 > 5)
            return ErrorCodes::ArgsError;
        return R();
    };

    auto err = f0(100);
    // operator==
    REQUIRE_EQ(err, ErrorCodes::ArgsError);
    REQUIRE_NE(err, R());
    // error_then
    err.error_then([&](ErrorCodes e) {
        REQUIRE(err.has_error());
        REQUIRE_FALSE(err.has_value());
        REQUIRE_EQ(e, ErrorCodes::ArgsError);
    });

    auto success = f0(4);
    success.and_then([&]() {
        REQUIRE(success.has_value());
        REQUIRE_FALSE(success.has_error());
    });
    REQUIRE(f0(5).has_value());
    REQUIRE(f0(3).has_value());
    // operator==
    REQUIRE_EQ(success, R());
    REQUIRE_NE(success, ErrorCodes::ArgsError);
}