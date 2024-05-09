#include "SkrCore/log.hpp"
#include "SkrTestFramework/framework.hpp"
#include "SkrRTTR/type_signature.hpp"

TEST_CASE("test type signature")
{
    using namespace skr::rttr;

    // TODO. test compare
    SUBCASE("test compare")
    {
        TypeSignatureTyped<const int&>  a;
        TypeSignatureTyped<const int*>  b;
        TypeSignatureTyped<const int&&> c;
        TypeSignatureTyped<int&>        d;

        REQUIRE_FALSE(a.view().equal(b, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(a.view().equal(c, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(a.view().equal(d, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(b.view().equal(c, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(b.view().equal(d, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(c.view().equal(d, ETypeSignatureCompareFlag::Strict));
    }

    // TODO. test normalize

    // TODO. test complex type

    // TODO. test function

    // TODO. test generic type

    TypeSignatureTyped<const int&> test;

    SKR_LOG_FMT_INFO(u8"raw: {}", test.view().to_string());

    test.view().normalize(ETypeSignatureNormalizeFlag::IgnoreConst);

    SKR_LOG_FMT_INFO(u8"ignore const: {}", test.view().to_string());

    test.view().normalize();

    SKR_LOG_FMT_INFO(u8"ignore ref: {}", test.view().to_string());

    TypeSignatureTyped<void (*)(int&, int*&, const int&)> test_function_pointer;
    SKR_LOG_FMT_INFO(u8"function pointer: {}", test_function_pointer.view().to_string());
}