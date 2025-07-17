#include "SkrTestFramework/framework.hpp"
#include "SkrV8/ts_define_exporter.hpp"
#include "SkrV8/v8_isolate.hpp"
#include "SkrV8/v8_context.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrCore/log.hpp"
#include "test_v8_types.hpp"

skr::V8Isolate isolate;

struct V8EnvGuard {
    V8EnvGuard()
    {
        skr::init_v8();
        isolate.init();
    }
    ~V8EnvGuard()
    {
        isolate.shutdown();
        skr::shutdown_v8();
    }
} _guard{};

TEST_CASE("test v8")
{
    using namespace skr;

    SUBCASE("basic object")
    {
        V8Context& context = *isolate.create_context();
        SKR_DEFER({ isolate.destroy_context(&context); });

        // register context
        context.build_global_export([](ScriptModule& module) {
            module.register_type<test_v8::BasicObject>(u8"");
            module.register_type<test_v8::InheritObject>(u8"");
        });

        // set global
        test_v8::BasicObject* obj = SkrNew<test_v8::BasicObject>();
        context.set_global(u8"test_obj", obj);

        // test method
        context.exec_script(u8"test_obj.test_method(114514)");
        REQUIRE_EQ(obj->test_method_v, 114514);

        // test field
        context.exec_script(u8"test_obj.test_field = 1919810");
        REQUIRE_EQ(obj->test_field, 1919810);
        obj->test_field = 44554;
        context.exec_script(u8"test_obj.test_method(test_obj.test_field)");
        REQUIRE_EQ(obj->test_method_v, 44554);

        // test static method
        context.exec_script(u8"BasicObject.test_static_method(1919810)");
        REQUIRE_EQ(test_v8::BasicObject::test_static_method_v, 1919810);

        // test static field
        context.exec_script(u8"BasicObject.test_static_field = 114514");
        REQUIRE_EQ(test_v8::BasicObject::test_static_field, 114514);
        test_v8::BasicObject::test_static_field = 44544;
        context.exec_script(u8"test_obj.test_method(Number(BasicObject.test_static_field))");
        REQUIRE_EQ(obj->test_method_v, 44544);

        // test property
        context.exec_script(u8"test_obj.test_prop = 1");
        context.exec_script(u8"test_obj.test_prop = 2");
        context.exec_script(u8"test_obj.test_prop = 3");
        context.exec_script(u8"test_obj.test_prop = 114514");
        REQUIRE_EQ(obj->test_prop, 114514);
        REQUIRE_EQ(obj->test_prop_set_count, 4);
        context.exec_script(u8"let prop_v = test_obj.test_prop");
        context.exec_script(u8"BasicObject.test_static_field = prop_v");
        REQUIRE_EQ(obj->test_prop_get_count, 1);
        REQUIRE_EQ(test_v8::BasicObject::test_static_field, 114514);
        obj->test_prop = 10086;
        context.exec_script(u8"test_obj.test_method(test_obj.test_prop)");
        REQUIRE_EQ(obj->test_method_v, 10086);
        REQUIRE_EQ(obj->test_prop_get_count, 2);

        // test new
        context.exec_script(u8"let new_obj_1 = new BasicObject()");
        REQUIRE_EQ(test_v8::BasicObject::test_ctor_value, 114514);

        // test inherit
        context.exec_script(u8"let inherit_obj = new InheritObject()");
        context.exec_script(u8"inherit_obj.inherit_method(114514)");
        REQUIRE_EQ(test_v8::BasicObject::test_ctor_value, 1919810);
        {
            auto result = context.exec_script(u8"inherit_obj.test_field");
            REQUIRE(!result.is_empty());
            auto v = result.get<uint32_t>();
            REQUIRE(v.has_value());
            REQUIRE_EQ(v.value(), 114514 * 100);
        }
        context.exec_script(u8"inherit_obj.test_method(114514)");
        context.exec_script(u8"inherit_obj.assign_method_v_to_field()");
        {
            auto result = context.exec_script(u8"inherit_obj.test_field");
            REQUIRE(!result.is_empty());
            auto v = result.get<uint32_t>();
            REQUIRE(v.has_value());
            REQUIRE_EQ(v.value(), 114514);
        }

        // test lifetime
        SkrDelete(obj);
        context.exec_script(u8R"__(
            let error = false
            try {
                test_obj.test_method(114514);
            } catch (e) {
                error = true;
            }
        )__");
        {
            auto result = context.exec_script(u8"error");
            REQUIRE(!result.is_empty());
            auto v = result.get<bool>();
            REQUIRE(v.has_value());
            REQUIRE(v.value() == true);
        }

        context.shutdown();
    }

    SUBCASE("basic value")
    {
        V8Context& context = *isolate.create_context();
        SKR_DEFER({ isolate.destroy_context(&context); });

        // register context
        context.build_global_export([](ScriptModule& module) {
            module.register_type<test_v8::BasicValue>(u8"");
            module.register_type<test_v8::InheritValue>(u8"");
        });

        // set global
        test_v8::BasicValue value{};
        context.set_global(u8"test_value", value);

        // test method
        {
            context.exec_script(u8"test_value.test_method(114514)");
            auto result = context.exec_script(u8"test_value");
            REQUIRE_EQ(result.get<test_v8::BasicValue>().value().test_method_v, 114514);
        }

        // test field
        {
            context.exec_script(u8"test_value.test_field = 1919810");
            auto result = context.exec_script(u8"test_value");
            REQUIRE_EQ(result.get<test_v8::BasicValue>().value().test_field, 1919810);
        }

        // test static method
        context.exec_script(u8"BasicValue.test_static_method(1919810)");
        REQUIRE_EQ(test_v8::BasicValue::test_static_method_v, 1919810);

        // test static field
        context.exec_script(u8"BasicValue.test_static_field = 114514");
        REQUIRE_EQ(test_v8::BasicValue::test_static_field, 114514);
        test_v8::BasicValue::test_static_field = 44544;
        {
            context.exec_script(u8"test_value.test_method(Number(BasicValue.test_static_field))");
            auto result = context.exec_script(u8"test_value");
            REQUIRE_EQ(result.get<test_v8::BasicValue>().value().test_method_v, 44544);
        }

        // test property
        {
            context.exec_script(u8"test_value.test_prop = 1");
            context.exec_script(u8"test_value.test_prop = 2");
            context.exec_script(u8"test_value.test_prop = 3");
            context.exec_script(u8"test_value.test_prop = 114514");
            auto result = context.exec_script(u8"test_value");
            REQUIRE_EQ(result.get<test_v8::BasicValue>().value().test_prop, 114514);
            REQUIRE_EQ(result.get<test_v8::BasicValue>().value().test_prop_set_count, 4);
        }
        {
            context.exec_script(u8"let prop_v = test_value.test_prop");
            context.exec_script(u8"BasicValue.test_static_field = prop_v");
            auto result = context.exec_script(u8"test_value");
            REQUIRE_EQ(result.get<test_v8::BasicValue>().value().test_prop_get_count, 1);
            REQUIRE_EQ(test_v8::BasicValue::test_static_field, 114514);
        }

        // test new
        {
            context.exec_script(u8"let new_value_1 = new BasicValue()");
            REQUIRE_EQ(test_v8::BasicValue::test_ctor_value, 114514);
        }

        // test inherit
        {
            context.exec_script(u8"let inherit_value = new InheritValue()");
            context.exec_script(u8"inherit_value.inherit_method(114514)");
            REQUIRE_EQ(test_v8::BasicValue::test_ctor_value, 1919810);
        }
        {
            auto result = context.exec_script(u8"inherit_value.test_field");
            REQUIRE(!result.is_empty());
            auto v = result.get<uint32_t>();
            REQUIRE(v.has_value());
            REQUIRE_EQ(v.value(), 114514 * 100);
        }
        context.exec_script(u8"inherit_value.test_method(114514)");
        context.exec_script(u8"inherit_value.assign_method_v_to_field()");
        {
            auto result = context.exec_script(u8"inherit_value.test_field");
            REQUIRE(!result.is_empty());
            auto v = result.get<uint32_t>();
            REQUIRE(v.has_value());
            REQUIRE_EQ(v.value(), 114514);
        }

        context.shutdown();
    }

    SUBCASE("basic mapping")
    {
        V8Context& context = *isolate.create_context();
        SKR_DEFER({ isolate.destroy_context(&context); });

        context.build_global_export([](ScriptModule& module) {
            // module.register_type<test_v8::BasicMapping>(u8"");    //!NOTE. mapping type need not to be registered
            // module.register_type<test_v8::InheritMapping>(u8"");  //!NOTE. mapping type need not to be registered
            module.register_type<test_v8::BasicMappingHelper>(u8"");
        });

        // test basic
        context.exec_script(u8"BasicMappingHelper.basic_value = {x:11, y:45, z: 14}");
        REQUIRE_EQ(test_v8::BasicMappingHelper::basic_value.x, 11);
        REQUIRE_EQ(test_v8::BasicMappingHelper::basic_value.y, 45);
        REQUIRE_EQ(test_v8::BasicMappingHelper::basic_value.z, 14);

        // test inherit
        context.exec_script(u8"BasicMappingHelper.inherit_value = {x:11, y:45, z: 14, w: 1564656}");
        REQUIRE_EQ(test_v8::BasicMappingHelper::inherit_value.x, 11);
        REQUIRE_EQ(test_v8::BasicMappingHelper::inherit_value.y, 45);
        REQUIRE_EQ(test_v8::BasicMappingHelper::inherit_value.z, 14);
        REQUIRE_EQ(test_v8::BasicMappingHelper::inherit_value.w, 1564656);

        // test pattern match
        context.exec_script(u8"BasicMappingHelper.basic_value = {x:1, y:2, z:3, w: 4}");
        REQUIRE_EQ(test_v8::BasicMappingHelper::basic_value.x, 1);
        REQUIRE_EQ(test_v8::BasicMappingHelper::basic_value.y, 2);
        REQUIRE_EQ(test_v8::BasicMappingHelper::basic_value.z, 3);

        context.shutdown();
    }

    SUBCASE("basic enum")
    {
        V8Context& context = *isolate.create_context();
        SKR_DEFER({ isolate.destroy_context(&context); });

        context.build_global_export([](ScriptModule& module) {
            module.register_type<test_v8::BasicEnum>(u8"");
            module.register_type<test_v8::BasicEnumHelper>(u8"");
        });

        // test assign
        context.exec_script(u8"BasicEnumHelper.test_value = BasicEnum.Value4");
        REQUIRE_EQ(test_v8::BasicEnumHelper::test_value, test_v8::BasicEnum::Value4);

        // test to string
        context.exec_script(u8"BasicEnumHelper.test_name = BasicEnum.to_string(BasicEnum.Value4)");
        REQUIRE_EQ(test_v8::BasicEnumHelper::test_name, u8"Value4");
        context.exec_script(u8"BasicEnumHelper.test_name = BasicEnum.to_string(8848)");
        REQUIRE_EQ(test_v8::BasicEnumHelper::test_name, u8"Value5");

        // test from string
        context.exec_script(u8"BasicEnumHelper.test_value = BasicEnum.from_string('Value3')");
        REQUIRE_EQ(test_v8::BasicEnumHelper::test_value, test_v8::BasicEnum::Value3);
        context.exec_script(u8"BasicEnumHelper.test_value = BasicEnum.from_string('Value5')");
        REQUIRE_EQ(test_v8::BasicEnumHelper::test_value, test_v8::BasicEnum::Value5);

        context.shutdown();
    }

    SUBCASE("param flag")
    {
        V8Context& context = *isolate.create_context();
        SKR_DEFER({ isolate.destroy_context(&context); });

        context.build_global_export([](ScriptModule& module) {
            module.register_type<test_v8::ParamFlagTest>(u8"");
            module.register_type<test_v8::ParamFlagTestValue>(u8"");
        });

        // test out & inout
        context.exec_script(u8"ParamFlagTest.test_value = ParamFlagTest.test_pure_out()");
        REQUIRE_EQ(test_v8::ParamFlagTest::test_value, u8"mambo");
        context.exec_script(u8"ParamFlagTest.test_value = ParamFlagTest.test_inout('mambo')");
        REQUIRE_EQ(test_v8::ParamFlagTest::test_value, u8"mambo mambo");

        // test multi out & inout
        context.exec_script(u8"let [out1, out2, out3] = ParamFlagTest.test_multi_out()");
        context.exec_script(u8"ParamFlagTest.test_value = `${out1} + ${out2} + ${out3}`");
        REQUIRE_EQ(test_v8::ParamFlagTest::test_value, u8"mamba + out + mambo");
        context.exec_script(u8"let [inout1, inout2, inout3] = ParamFlagTest.test_multi_inout('AAA', 'BBB')");
        context.exec_script(u8"ParamFlagTest.test_value = `${inout1} + ${inout2} + ${inout3}`");
        REQUIRE_EQ(test_v8::ParamFlagTest::test_value, u8"mamba + AAAout + BBBmambo");

        // test value out & inout
        {
            // test out
            auto result = context.exec_script(u8"ParamFlagTest.test_value_pure_out()");
            REQUIRE_EQ(result.get<test_v8::ParamFlagTestValue>().value().value, u8"mambo");

            // test inout
            result = context.exec_script(u8R"__(
                let test_value = new ParamFlagTestValue()
                test_value.value = 'ohhhh'
                ParamFlagTest.test_value_inout(test_value)
                test_value
            )__");
            REQUIRE_EQ(result.get<test_v8::ParamFlagTestValue>().value().value, u8"ohhhh baka");
        }

        context.shutdown();
    }

    SUBCASE("string")
    {
        V8Context& context = *isolate.create_context();
        SKR_DEFER({ isolate.destroy_context(&context); });

        context.build_global_export([](ScriptModule& module) {
            module.register_type<test_v8::TestString>(u8"");
        });

        // test get str
        context.exec_script(u8"TestString.value = TestString.get_str()");
        REQUIRE_EQ(test_v8::TestString::value, u8"mamba out");

        // test set str
        context.exec_script(u8"TestString.set_str('牢大')");
        REQUIRE_EQ(test_v8::TestString::value, u8"牢大");

        // test get view
        {
            auto result = context.exec_script(u8"TestString.get_view()");
            REQUIRE_EQ(result.get<skr::String>().value(), u8"牢大");
        }

        // test set view
        context.exec_script(u8"TestString.set_view('牢大出击')");
        REQUIRE_EQ(test_v8::TestString::value, u8"牢大出击");

        // test prop
        context.exec_script(u8"TestString.prop_value = `oh yeah ${TestString.prop_value}`");
        REQUIRE_EQ(test_v8::TestString::value, u8"oh yeah 牢大出击");

        context.shutdown();
    }

    SUBCASE("call script")
    {
        V8Context& context = *isolate.create_context();
        SKR_DEFER({ isolate.destroy_context(&context); });
        context.build_global_export([](ScriptModule& module) {
        });

        context.exec_script(u8R"__(
            function str_join(a, b, c) {
                return `${a} + ${b} + ${c}`
            }
        )__");
        auto str_join_func = context.get_global(u8"str_join");
        auto result        = str_join_func.call<skr::String>(skr::String{ u8"hello" }, skr::String{ u8"world" }, skr::String{ u8"!" });
        REQUIRE(result.has_value());
        REQUIRE_EQ(result.value(), u8"hello + world + !");

        context.shutdown();
    }

    SUBCASE("rttr mixin")
    {
        V8Context& context = *isolate.create_context();
        SKR_DEFER({ isolate.destroy_context(&context); });
        context.build_global_export([](ScriptModule& module) {
            module.register_type<test_v8::TestMixinValue>(u8"");
            module.register_type<test_v8::RttrMixin>(u8"");
            module.register_type<test_v8::MixinHelper>(u8"");
        });

        v8::HandleScope    handle_scope(isolate.v8_isolate());
        v8::Context::Scope context_scope(context.v8_context().Get(isolate.v8_isolate()));

        // no mixin
        {
            context.exec_script(u8R"__(
                let native = new RttrMixin()
                MixinHelper.mixin = native
            )__");

            REQUIRE(test_v8::MixinHelper::mixin != nullptr);
            REQUIRE_EQ(test_v8::MixinHelper::mixin->test_mixin_ret(), u8"FUCK");
            test_v8::MixinHelper::mixin->test_mixin_param(114514);
            REQUIRE_EQ(test_v8::MixinHelper::mixin->test_mixin_value, 114514 * 2);
        }

        // class mixin
        {
            context.exec_script(u8R"__(
                class MixInImpl extends RttrMixin {
                    constructor() {
                        super();
                    }
                    test_mixin_ret() {
                        return `${super.test_mixin_ret()} + mixin`;
                    }
                    test_mixin_param(v) {
                        super.test_mixin_param(v * BigInt(5));
                    }
                }
                let mixin = new MixInImpl()
                MixinHelper.mixin = mixin
            )__");

            REQUIRE(test_v8::MixinHelper::mixin != nullptr);
            REQUIRE_EQ(test_v8::MixinHelper::mixin->test_mixin_ret(), u8"FUCK + mixin");
            test_v8::MixinHelper::mixin->test_mixin_param(114514);
            REQUIRE_EQ(test_v8::MixinHelper::mixin->test_mixin_value, 114514 * 6);
        }

        // object mixin
        {
            context.exec_script(u8R"__(
                let object_mixin = new RttrMixin()
                const raw_mixin_ret = object_mixin.test_mixin_ret
                const raw_mixin_param = object_mixin.test_mixin_param

                object_mixin.test_mixin_ret = function() {
                    return `${raw_mixin_ret.call(this)} + mixin`;
                }
                object_mixin.test_mixin_param = function(v) {
                    raw_mixin_param.call(this, v * BigInt(5));
                }

                MixinHelper.mixin = object_mixin
            )__");

            REQUIRE(test_v8::MixinHelper::mixin != nullptr);
            REQUIRE_EQ(test_v8::MixinHelper::mixin->test_mixin_ret(), u8"FUCK + mixin");
            test_v8::MixinHelper::mixin->test_mixin_param(114514);
            REQUIRE_EQ(test_v8::MixinHelper::mixin->test_mixin_value, 114514 * 6);
        }

        // test value
        {
            context.exec_script(u8R"__(
                let test_value = new RttrMixin()
                MixinHelper.mixin = test_value
            )__");

            // no mixin
            test_v8::TestMixinValue test, test_b;
            test.name = u8"TEST_";
            test_v8::MixinHelper::mixin->test_inout_value(test);
            REQUIRE_EQ(test.name, u8"TEST_INOUT_VALUE");
            test.name = u8"INVALID";
            test_v8::MixinHelper::mixin->test_pure_out_value(test);
            REQUIRE_EQ(test.name, u8"PURE_OUT_VALUE");
            test.name = u8"INVALID";
            test      = test_v8::MixinHelper::mixin->test_return_value();
            REQUIRE_EQ(test.name, u8"RETURN_VALUE");
            test.name                 = u8"INVALID";
            test_b.name               = u8"INVALID";
            skr::String multi_out_ret = test_v8::MixinHelper::mixin->test_multi_out_value(test, test_b);
            REQUIRE_EQ(multi_out_ret, u8"MULTI_OUT");
            REQUIRE_EQ(test.name, u8"OUT_VALUE_1");
            REQUIRE_EQ(test_b.name, u8"OUT_VALUE_2");

            // apply mixin
            context.exec_script(u8R"__(
                let old_test_input_value = test_value.test_inout_value
                let old_test_pure_out_value = test_value.test_pure_out_value
                let old_test_return_value = test_value.test_return_value
                let old_test_multi_out_value = test_value.test_multi_out_value
                let test_out_scope_param = undefined

                test_value.test_inout_value = function(v) {
                    old_test_input_value.call(this, v)
                    v.name += ' + mixin'
                    test_out_scope_param = v
                }
                test_value.test_pure_out_value = function() {
                    let v = old_test_pure_out_value.call(this)
                    v.name += ' + mixin'
                    return v
                }
                test_value.test_return_value = function() {
                    let v = old_test_return_value.call(this)
                    v.name += ' + mixin'
                    return v
                }
                test_value.test_multi_out_value = function() {
                    return [
                        "MULTI_OUT_FROM_JS",
                        new TestMixinValue('OUT_VALUE_1 + mixin'),
                        new TestMixinValue('OUT_VALUE_2 + mixin')
                    ]
                }
            )__");

            // with mixin
            test.name = u8"TEST_";
            test_v8::MixinHelper::mixin->test_inout_value(test);
            REQUIRE_EQ(test.name, u8"TEST_INOUT_VALUE + mixin");
            test.name = u8"INVALID";
            test_v8::MixinHelper::mixin->test_pure_out_value(test);
            REQUIRE_EQ(test.name, u8"PURE_OUT_VALUE + mixin");
            test.name = u8"INVALID";
            test      = test_v8::MixinHelper::mixin->test_return_value();
            REQUIRE_EQ(test.name, u8"RETURN_VALUE + mixin");
            test.name     = u8"INVALID";
            test_b.name   = u8"INVALID";
            multi_out_ret = test_v8::MixinHelper::mixin->test_multi_out_value(test, test_b);
            REQUIRE_EQ(multi_out_ret, u8"MULTI_OUT_FROM_JS");
            REQUIRE_EQ(test.name, u8"OUT_VALUE_1 + mixin");
            REQUIRE_EQ(test_b.name, u8"OUT_VALUE_2 + mixin");

            // test use out scope param
            auto call_out_scope_result = context.exec_script(u8R"__(
                let error = false
                try {
                    test_out_scope_param.name = 'fuck u'
                } catch (e) {
                    error = true
                }
                error
            )__");
            REQUIRE(call_out_scope_result.get<bool>().has_value());
            REQUIRE_EQ(call_out_scope_result.get<bool>().value(), true);
        }

        context.shutdown();
    }

    // output .d.ts
    SUBCASE("output d.ts")
    {
        // build module
        ScriptBinderManager bm;
        ScriptModule        sm;
        sm.manager     = &bm;
        uint32_t count = 0;
        each_types_of_module(u8"V8Test", [&](const RTTRType* type) -> bool {
            if (count % 2 == 0)
            {
                sm.register_type(type);
            }
            else
            {
                sm.register_type(type, u8"fuck::you::圆头");
            }

            ++count;
            return true;
        });
        sm.check_full_export();

        // export module
        TSDefineExporter exporter;
        exporter.module = &sm;
        auto result     = exporter.generate_module(u8"圆头");

        // write to file
        auto file = fopen("test_v8.d.ts", "wb");
        if (file)
        {
            fwrite(result.c_str(), 1, result.size(), file);
            fclose(file);
        }
        else
        {
            SKR_LOG_ERROR(u8"failed to open test_v8.d.ts");
        }
    }
}