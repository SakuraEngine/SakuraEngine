#pragma once
#include "SkrCore/log.hpp"
#include <SkrRTTR/script/scriptble_object.hpp>
#include "test_v8_types.generated.h"

// error export test
namespace test_v8
{
struct [[sscript_visible, sscript_newable, sattr(guid = "a1b2c3d4-e5f6-7890-abcd-ef1234567890")]]
ErrorExport
{
    [[sscript_visible]]
    int32_t* primitive_ref_field = {};

    [[sscript_visible]]
    skr::StringView string_view_field = {};

    [[sscript_visible]]
    ErrorExport()
    {
    }

    [[sscript_visible]]
    ErrorExport(int32_t a)
    {
    }

    [[sscript_visible]]
    void primitive_pointer_param(int32_t* a)
    {
    }
};
} // namespace test_v8

// basic object
namespace test_v8
{
struct [[sscript_visible, sscript_newable, sattr(guid = "9099f452-beab-482b-a9c8-0f582bd7f5b4" rttr = @full)]]
BasicObject : public ::skr::ScriptbleObject
{
    SKR_GENERATE_BODY(BasicObject)

    [[sscript_visible]]
    BasicObject()
    {
        test_ctor_value = 114514;
    }

    static int32_t test_ctor_value;

    // method
    [[sscript_visible]]
    void test_method(int32_t v)
    {
        test_method_v = v;
    }
    int32_t test_method_v = 0;

    // field
    [[sscript_visible]]
    void assign_method_v_to_field()
    {
        test_field = test_method_v;
    }
    [[sscript_visible]]
    uint32_t test_field = 0;

    // static method
    [[sscript_visible]]
    static void test_static_method(int64_t v)
    {
        test_static_method_v = v;
    }
    static int64_t test_static_method_v;

    // static field
    [[sscript_visible]]
    static uint64_t test_static_field;

    // test property
    [[sscript_visible, sscript_getter(test_prop)]]
    int32_t get_test_prop()
    {
        ++test_prop_get_count;
        return test_prop;
    }
    [[sscript_visible, sscript_setter(test_prop)]]
    void set_test_prop(int32_t v)
    {
        ++test_prop_set_count;
        test_prop = v;
    }
    int32_t test_prop = 0;
    int32_t test_prop_get_count = 0;
    int32_t test_prop_set_count = 0;
};

struct [[sscript_visible, sscript_newable, sattr(guid = "549b208d-ca2b-44e1-a705-ef95e0c607b5" rttr = @full)]]
InheritObject : public BasicObject
{
    SKR_GENERATE_BODY(InheritObject)

    [[sscript_visible]]
    InheritObject()
    {
        test_ctor_value = 1919810;
    }

    [[sscript_visible]]
    void inherit_method(int32_t v)
    {
        test_field = v * 100;
    }
};
} // namespace test_v8

// basic value
namespace test_v8
{
struct [[sscript_visible, sscript_newable, sattr(guid = "ecaf33a3-0c3f-46b5-87aa-ba895fa6c38f" rttr = @full)]]
BasicValue
{
    [[sscript_visible]]
    BasicValue()
    {
        test_ctor_value = 114514;
    }

    static int32_t test_ctor_value;

    // method
    [[sscript_visible]]
    void test_method(int32_t v)
    {
        test_method_v = v;
    }
    int32_t test_method_v = 0;

    // field
    [[sscript_visible]]
    void assign_method_v_to_field()
    {
        test_field = test_method_v;
    }
    [[sscript_visible]]
    uint32_t test_field = 0;

    // static method
    [[sscript_visible]]
    static void test_static_method(int64_t v)
    {
        test_static_method_v = v;
    }
    static int64_t test_static_method_v;

    // static field
    [[sscript_visible]]
    static uint64_t test_static_field;

    // test property
    [[sscript_visible, sscript_getter(test_prop)]]
    int32_t get_test_prop()
    {
        ++test_prop_get_count;
        return test_prop;
    }
    [[sscript_visible, sscript_setter(test_prop)]]
    void set_test_prop(int32_t v)
    {
        ++test_prop_set_count;
        test_prop = v;
    }
    int32_t test_prop = 0;
    int32_t test_prop_get_count = 0;
    int32_t test_prop_set_count = 0;
};

struct [[sscript_visible, sscript_newable, sattr(guid = "2aeb519b-9b8e-4561-b185-81429dd1dd2e" rttr = @full)]]
InheritValue : public BasicValue
{
    [[sscript_visible]]
    InheritValue()
    {
        test_ctor_value = 1919810;
    }

    [[sscript_visible]]
    void inherit_method(int32_t v)
    {
        test_field = v * 100;
    }
};
} // namespace test_v8

// basic mapping
namespace test_v8
{
struct [[sscript_visible, sscript_mapping, sattr(guid = "3fcd040d-e180-4802-996d-d67417e45f7e" rttr = @full)]]
BasicMapping
{
    [[sscript_visible]]
    double x = 0;
    [[sscript_visible]]
    double y = 0;
    [[sscript_visible]]
    double z = 0;
};
struct [[sscript_visible, sscript_mapping, sattr(guid = "b9b953e3-b63b-49bf-8d34-2dba7ff49dd2" rttr = @full)]]
InheritMapping : BasicMapping
{
    [[sscript_visible]]
    double w = 0;
};
struct [[sscript_visible, sattr(guid = "06e9fa99-9432-43fa-ba55-d22737a7de60" rttr = @full)]]
BasicMappingHelper
{
    [[sscript_visible]]
    static BasicMapping basic_value;
    [[sscript_visible]]
    static InheritMapping inherit_value;
};
} // namespace test_v8

// basic enum
namespace test_v8
{
enum class [[sscript_visible, sattr(guid = "56611484-25f9-491b-b85e-08f634839938")]] BasicEnum : int32_t
{
    Value1 [[sscript_visible]] = 0,
    Value2 [[sscript_visible]] = 1,
    Value3 [[sscript_visible]] = 114514,
    Value4 [[sscript_visible]] = 3,
    Value5 [[sscript_visible]] = 8848,
};

struct [[sscript_visible, sattr(guid = "26f991d3-7030-4b04-af12-0e1f0d93bba6" rttr = @full)]]
BasicEnumHelper
{
    [[sscript_visible]]
    static BasicEnum test_value;
    [[sscript_visible]]
    static skr::String test_name;
};
} // namespace test_v8

// param flag
namespace test_v8
{
struct [[sscript_visible, sscript_newable, sattr(guid = "7d514abe-a7f9-4512-a94f-4f3e9eb19d94")]]
ParamFlagTestValue
{
    [[sscript_visible]]
    ParamFlagTestValue() = default;

    [[sscript_visible]]
    skr::String value;
};

struct [[sscript_visible, sattr(guid = "d74058a0-24c4-4f86-b062-299674e3df53")]]
ParamFlagTest
{
    [[sscript_visible]]
    static skr::String test_value;

    [[sscript_visible]]
    static void test_pure_out([[sparam_out]] skr::String& v)
    {
        v = u8"mambo";
    }

    [[sscript_visible]]
    static void test_inout([[sparam_inout]] skr::String& v)
    {
        v.append(u8" mambo");
    }

    [[sscript_visible]]
    static skr::String test_multi_out([[sparam_out]] skr::String& v1, [[sparam_out]] skr::String& v2)
    {
        v1 = u8"out";
        v2 = u8"mambo";
        return u8"mamba";
    }
    [[sscript_visible]]
    static skr::String test_multi_inout([[sparam_inout]] skr::String& v1, [[sparam_inout]] skr::String& v2)
    {
        v1.append(u8"out");
        v2.append(u8"mambo");
        return u8"mamba";
    }

    [[sscript_visible]]
    static void test_value_pure_out([[sparam_out]] ParamFlagTestValue& v)
    {
        v.value = u8"mambo";
    }
    [[sscript_visible]]
    static void test_value_inout([[sparam_inout]] ParamFlagTestValue& v)
    {
        v.value.append(u8" baka");
    }
};
} // namespace test_v8

// test string
namespace test_v8
{
struct [[sscript_visible, sattr(guid = "e24c4d8f-5294-4c91-b7ad-f1f56c140c8b")]]
TestString
{
    [[sscript_visible]]
    static skr::String get_str()
    {
        return u8"mamba out";
    }
    [[sscript_visible]]
    static void set_str(const skr::String& v)
    {
        value = v;
    }

    [[sscript_visible]]
    static void set_view(skr::StringView v)
    {
        value = skr::String(v);
    }

    [[sscript_visible, sscript_getter(prop_value)]]
    static skr::String prop_value()
    {
        return value;
    }
    [[sscript_visible, sscript_setter(prop_value)]]
    static void set_prop_value(skr::StringView v)
    {
        value = skr::String(v);
    }

    [[sscript_visible]]
    static skr::String value;
};
} // namespace test_v8

// mixin
namespace test_v8
{
struct [[sscript_visible, sscript_newable, sattr(guid = "fe94dd7f-9c4e-4cb3-a275-0f8f8011145c" rttr = @full)]]
TestMixinValue
{
    TestMixinValue() = default;
    [[sscript_visible]]
    TestMixinValue(skr::String v)
        : name(v)
    {
    }

    [[sscript_visible]]
    skr::String name;
};

struct [[sscript_visible, sscript_newable, sattr(guid = "f67c4345-e723-4597-9e88-1e48564b130d" rttr = @full)]]
RttrMixin : skr::ScriptbleObject
{
    SKR_GENERATE_BODY(RttrMixin)

    [[sscript_visible]]
    RttrMixin() = default;

    [[sscript_visible, sscript_mixin]]
    skr::String test_mixin_ret();
    skr::String test_mixin_ret_impl() { return u8"FUCK"; }

    [[sscript_visible, sscript_mixin]]
    void test_mixin_param(uint64_t v);
    void test_mixin_param_impl(uint64_t v)
    {
        test_mixin_value = v + 114514;
    }

    [[sscript_visible, sscript_mixin]]
    void test_inout_value(TestMixinValue& v);
    void test_inout_value_impl(TestMixinValue& v)
    {
        v.name.append(u8"INOUT_VALUE");
    }

    [[sscript_visible, sscript_mixin]]
    void test_pure_out_value([[sparam_out]] TestMixinValue& v);
    void test_pure_out_value_impl([[sparam_out]] TestMixinValue& v)
    {
        v.name = u8"PURE_OUT_VALUE";
    }

    [[sscript_visible, sscript_mixin]]
    skr::String test_multi_out_value([[sparam_out]] TestMixinValue& v1, [[sparam_out]] TestMixinValue& v2);
    skr::String test_multi_out_value_impl([[sparam_out]] TestMixinValue& v1, [[sparam_out]] TestMixinValue& v2)
    {
        v1.name = u8"OUT_VALUE_1";
        v2.name = u8"OUT_VALUE_2";
        return u8"MULTI_OUT";
    }

    [[sscript_visible, sscript_mixin]]
    TestMixinValue test_return_value();
    TestMixinValue test_return_value_impl()
    {
        return TestMixinValue(u8"RETURN_VALUE");
    }

    [[sscript_visible]]
    uint64_t test_mixin_value = 0;
};

struct [[sscript_visible, sattr(guid = "3d784c7b-0333-44d6-9759-3724d32f9d70" rttr = @full)]]
MixinHelper
{
    [[sscript_visible]]
    static RttrMixin* mixin;

    [[sscript_visible]]
    static TestMixinValue test_value;
};

} // namespace test_v8

// function ref
namespace test_v8
{
struct [[sscript_visible, sscript_newable, sattr(guid = "fbf0c81b-b2ba-4105-a8fd-0a3744c36d25" rttr = @full)]]
TestFunctionRef
{
    [[sscript_visible]]
    static void range_map(
        uint32_t start,
        uint32_t end,
        uint32_t gap,
        skr::FunctionRef<skr::String(uint32_t)> func)
    {
        if (func)
        {
            for (uint32_t i = start; i < end; i += gap)
            {
                SKR_LOG_FMT_INFO(u8"{} => {}", i, func(i));
            }
        }
    }
};
} // namespace test_v8