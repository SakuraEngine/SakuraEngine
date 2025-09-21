#include "SkrTestFramework/framework.hpp"
#include "test_rttr_types.hpp"

TEST_CASE("test type signature")
{
    using namespace skr;

    auto type = type_of<test_rttr::DynamicStackInvoke>();

    SUBCASE("static method")
    {
        SUBCASE("direct call")
        {
            const auto&  method = type->find_static_method({ .name = { u8"add" } });
            DynamicStack stack;

            // combine params
            stack.add_param<int32_t>(1);
            stack.add_param<int32_t>(1);

            // invoke
            stack.return_behaviour_store();
            method->dynamic_stack_invoke(stack);

            // read return
            auto result = stack.get_return<int32_t>();
            REQUIRE(result == 2);
        }

        SUBCASE("grab return use ref")
        {
            const auto&  method = type->find_static_method({ .name = { u8"add" } });
            DynamicStack stack;

            // combine params
            stack.add_param<int32_t>(1);
            stack.add_param<int32_t>(1);

            // invoke
            int32_t result;
            stack.return_behaviour_store_to_ref(&result);
            method->dynamic_stack_invoke(stack);
            REQUIRE(result == 2);
        }

        SUBCASE("ref call")
        {
            const auto&  method = type->find_static_method({ .name = { u8"add" } });
            DynamicStack stack;

            int32_t a, b;

            // combine params
            stack.add_param<int32_t*>(&a, EDynamicStackParamKind::Reference);
            stack.add_param<int32_t*>(&b, EDynamicStackParamKind::Reference);

            // assign value
            a = 114;
            b = 514;

            // invoke
            int32_t result;
            stack.return_behaviour_store_to_ref(&result);
            method->dynamic_stack_invoke(stack);
            REQUIRE(result == (114 + 514));
        }

        SUBCASE("direct call(ref)")
        {
            const auto&  method = type->find_static_method({ .name = { u8"add_ref" } });
            DynamicStack stack;

            int32_t a, b;

            // combine params
            stack.add_param<int32_t*>(&a);
            stack.add_param<int32_t*>(&b);

            // setup value
            a = 114;
            b = 514;

            // invoke
            stack.return_behaviour_store();
            method->dynamic_stack_invoke(stack);

            // read return
            auto result = stack.get_return<int32_t>();
            REQUIRE(result == (114 + 514));
        }

        SUBCASE("xvalue call")
        {
            const auto&  method = type->find_static_method({ .name = { u8"add_ref" } });
            DynamicStack stack;

            // combine params
            stack.add_param<int32_t>(2, EDynamicStackParamKind::XValue);
            stack.add_param<int32_t>(3, EDynamicStackParamKind::XValue);

            // invoke
            stack.return_behaviour_store();
            method->dynamic_stack_invoke(stack);

            // read return
            auto result = stack.get_return<int32_t>();
            REQUIRE(result == 5);
        }

        SUBCASE("pass ref")
        {
            const auto&  method = type->find_static_method({ .name = { u8"add_use_out_param" } });
            DynamicStack stack;

            int32_t out;

            // combine params
            stack.add_param<int32_t>(1);
            stack.add_param<int32_t>(2);
            stack.add_param<int32_t*>(&out);

            // invoke
            method->dynamic_stack_invoke(stack);
            REQUIRE(out == 3);
        }
    }
}