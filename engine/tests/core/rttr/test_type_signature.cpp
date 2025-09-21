#include "SkrCore/log.hpp"
#include "SkrTestFramework/framework.hpp"
#include "SkrRTTR/type_signature.hpp"
#include "./test_rttr_types.hpp"
#include <SkrContainers/map.hpp>
#include <SkrContainers/optional.hpp>

TEST_CASE("test type signature")
{
    using namespace skr;

    SUBCASE("basic test")
    {
        TypeSignatureTyped<const int&>       type;
        TypeSignatureTyped<void(int, float)> func;

        REQUIRE(type.view().is_complete());
        REQUIRE(type.view().is_type());

        REQUIRE(func.view().is_complete());
        REQUIRE(func.view().is_function());
    }

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

    // test decay
    SUBCASE("test decay")
    {
        TypeSignatureTyped<const int&>  a;
        TypeSignatureTyped<const int*>  b;
        TypeSignatureTyped<const int&&> c;
        TypeSignatureTyped<int&>        d;

        // strict
        {
            TypeSignatureTyped<const int&>  decay_a;
            TypeSignatureTyped<const int*>  decay_b;
            TypeSignatureTyped<const int&&> decay_c;
            TypeSignatureTyped<int&>        decay_d;

            decay_a.decay(ETypeSignatureDecayFlag::Strict);
            decay_b.decay(ETypeSignatureDecayFlag::Strict);
            decay_c.decay(ETypeSignatureDecayFlag::Strict);
            decay_d.decay(ETypeSignatureDecayFlag::Strict);

            REQUIRE(decay_a.view().equal(a, ETypeSignatureCompareFlag::Strict));
            REQUIRE(decay_b.view().equal(b, ETypeSignatureCompareFlag::Strict));
            REQUIRE(decay_c.view().equal(c, ETypeSignatureCompareFlag::Strict));
            REQUIRE(decay_d.view().equal(d, ETypeSignatureCompareFlag::Strict));
        }

        // relax
        {
            TypeSignatureTyped<const int&>  decay_a;
            TypeSignatureTyped<const int*>  decay_b;
            TypeSignatureTyped<const int&&> decay_c;
            TypeSignatureTyped<int&>        decay_d;

            decay_a.decay(ETypeSignatureDecayFlag::Relax);
            decay_b.decay(ETypeSignatureDecayFlag::Relax);
            decay_c.decay(ETypeSignatureDecayFlag::Relax);
            decay_d.decay(ETypeSignatureDecayFlag::Relax);

            TypeSignatureTyped<int*> relax_result;
            REQUIRE(decay_a.view().equal(relax_result, ETypeSignatureCompareFlag::Strict));
            REQUIRE(decay_b.view().equal(relax_result, ETypeSignatureCompareFlag::Strict));
            REQUIRE(decay_c.view().equal(relax_result, ETypeSignatureCompareFlag::Strict));
            REQUIRE(decay_d.view().equal(relax_result, ETypeSignatureCompareFlag::Strict));
        }

        // decay ref to pointer
        {
            TypeSignatureTyped<const int&> decay_a;
            decay_a.decay(ETypeSignatureDecayFlag::RefAsPointer);

            TypeSignatureTyped<const int*> decay_result;
            REQUIRE(decay_a.view().equal(decay_result, ETypeSignatureCompareFlag::Strict));
        }

        // decay rvalue ref to pointer
        {
            TypeSignatureTyped<const int&&> decay_c;
            decay_c.decay(ETypeSignatureDecayFlag::RValueRefAsPointer);

            TypeSignatureTyped<const int*> decay_result;
            REQUIRE(decay_c.view().equal(decay_result, ETypeSignatureCompareFlag::Strict));
        }

        // decay rvalue ref to ref
        {
            TypeSignatureTyped<const int&&> decay_c;
            decay_c.decay(ETypeSignatureDecayFlag::IgnoreRvalue);

            TypeSignatureTyped<const int&> decay_result;
            REQUIRE(decay_c.view().equal(decay_result, ETypeSignatureCompareFlag::Strict));
        }
    }

    // test function
    SUBCASE("test function signature")
    {
        TypeSignatureTyped<void(int, const int&, int*, int&&)>     func;
        TypeSignatureTyped<void (*)(int, const int&, int*, int&&)> func_pointer;

        // basic test
        REQUIRE(func.view().is_complete());
        REQUIRE(func.view().is_function());
        REQUIRE(func_pointer.view().is_complete());
        REQUIRE(func_pointer.view().is_function());
        REQUIRE_FALSE(func.view().equal(func_pointer, ETypeSignatureCompareFlag::Strict));

        // test compare
        {
            TypeSignatureTyped<void(int, int*, int*, int*)> decay_func;

            REQUIRE_FALSE(decay_func.view().equal(func, ETypeSignatureCompareFlag::Strict));
            REQUIRE(decay_func.view().equal(func, ETypeSignatureCompareFlag::Relax));
        }

        // test split type
        {
            TypeSignatureTyped<void>       ret;
            TypeSignatureTyped<int>        param1;
            TypeSignatureTyped<const int&> param2;
            TypeSignatureTyped<int*>       param3;
            TypeSignatureTyped<int&&>      param4;

            auto view = func_pointer.view();
            view.jump_modifier();
            REQUIRE(view.is_function());
            REQUIRE(view.is_complete());
            REQUIRE(view.equal(func, ETypeSignatureCompareFlag::Strict));

            view.jump_next_data();
            auto ret_view    = view.jump_next_type_or_data();
            auto param1_view = view.jump_next_type_or_data();
            auto param2_view = view.jump_next_type_or_data();
            auto param3_view = view.jump_next_type_or_data();
            auto param4_view = view.jump_next_type_or_data();

            REQUIRE(ret_view.is_complete());
            REQUIRE(param1_view.is_complete());
            REQUIRE(param2_view.is_complete());
            REQUIRE(param3_view.is_complete());
            REQUIRE(param4_view.is_complete());

            REQUIRE(ret_view.equal(ret, ETypeSignatureCompareFlag::Strict));
            REQUIRE(param1_view.equal(param1, ETypeSignatureCompareFlag::Strict));
            REQUIRE(param2_view.equal(param2, ETypeSignatureCompareFlag::Strict));
            REQUIRE(param3_view.equal(param3, ETypeSignatureCompareFlag::Strict));
            REQUIRE(param4_view.equal(param4, ETypeSignatureCompareFlag::Strict));
        }
    }

    // test function/method/field signature
    SUBCASE("test function/method/field signature")
    {
        TypeSignatureTyped<void (*)(int, const int&, int*, int&&)> reference_func;
        TypeSignatureTyped<float>                                  reference_type;

        struct Test {
            void        method(int, const int&, int*, int&&) {}
            static void function(int, const int&, int*, int&&) {}

            float field;
        };

        auto method = type_signature_of_method_typed<&Test::method>();
        auto func   = type_signature_of_function_typed<&Test::function>();
        auto field  = type_signature_of_field_typed<&Test::field>();

        REQUIRE(method.view().is_complete());
        REQUIRE(method.view().is_function());
        REQUIRE(func.view().is_complete());

        REQUIRE(method.view().equal(reference_func, ETypeSignatureCompareFlag::Strict));
        REQUIRE(func.view().equal(reference_func, ETypeSignatureCompareFlag::Strict));
        REQUIRE(field.view().equal(reference_type, ETypeSignatureCompareFlag::Strict));
    }

    // test to string
    SUBCASE("to string")
    {
        TypeSignatureTyped<const int32_t&>                                 type;
        TypeSignatureTyped<void(int32_t, float)>                           func;
        TypeSignatureTyped<Optional<int32_t>>                              generic_type;
        TypeSignatureTyped<Map<String, Optional<test_rttr::TestDerivedB>>> generic_nested;

        SKR_LOG_FMT_INFO(u8"Type: {}", type.view());
        SKR_LOG_FMT_INFO(u8"Function: {}", func.view());
        SKR_LOG_FMT_INFO(u8"Generic Type: {}", generic_type.view());
        SKR_LOG_FMT_INFO(u8"Generic Map: {}", generic_nested.view());
    }
}
