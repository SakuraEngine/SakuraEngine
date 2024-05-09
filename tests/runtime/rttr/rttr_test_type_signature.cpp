#include "SkrCore/log.hpp"
#include "SkrTestFramework/framework.hpp"
#include "SkrRTTR/type_signature.hpp"

TEST_CASE("test type signature")
{
    using namespace skr::rttr;

    SUBCASE("test compare")
    {
        TypeSignatureTyped<const int&>  a;
        TypeSignatureTyped<const int*>  b;
        TypeSignatureTyped<const int&&> c;
        TypeSignatureTyped<int&>        d;

        // strict
        REQUIRE_FALSE(a.view().equal(b, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(a.view().equal(c, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(a.view().equal(d, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(b.view().equal(c, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(b.view().equal(d, ETypeSignatureCompareFlag::Strict));
        REQUIRE_FALSE(c.view().equal(d, ETypeSignatureCompareFlag::Strict));

        // relax
        REQUIRE(a.view().equal(b, ETypeSignatureCompareFlag::Relax));
        REQUIRE(a.view().equal(c, ETypeSignatureCompareFlag::Relax));
        REQUIRE(a.view().equal(d, ETypeSignatureCompareFlag::Relax));
        REQUIRE(b.view().equal(c, ETypeSignatureCompareFlag::Relax));
        REQUIRE(b.view().equal(d, ETypeSignatureCompareFlag::Relax));
        REQUIRE(c.view().equal(d, ETypeSignatureCompareFlag::Relax));

        // all ref as pointer
        REQUIRE(a.view().equal(b, ETypeSignatureCompareFlag::AllRefAsPointer));
        REQUIRE(a.view().equal(c, ETypeSignatureCompareFlag::AllRefAsPointer));
        REQUIRE(b.view().equal(c, ETypeSignatureCompareFlag::AllRefAsPointer));

        // ignore const
        REQUIRE(a.view().equal(d, ETypeSignatureCompareFlag::IgnoreConst));

        // ignore rvalue
        REQUIRE(a.view().equal(c, ETypeSignatureCompareFlag::IgnoreRValue));

        // ref as pointer
        REQUIRE(b.view().equal(a, ETypeSignatureCompareFlag::RefAsPointer));

        // rvalue ref as pointer
        REQUIRE(b.view().equal(c, ETypeSignatureCompareFlag::RValueRefAsPointer));

        // only ref as pointer
        REQUIRE(b.view().equal(a, ETypeSignatureCompareFlag::RefAsPointer | ETypeSignatureCompareFlag::RValueRefAsPointer));
        REQUIRE(b.view().equal(c, ETypeSignatureCompareFlag::RefAsPointer | ETypeSignatureCompareFlag::RValueRefAsPointer));
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