#pragma once
#include "SkrRT/config.h"
#include "SkrRTTR/iobject.hpp"
#if !defined(__meta__)
    #include "RTTRTest/test_rttr_types.generated.h"
#endif

// TODO. test invoke & field visit
// TODO. default value
// TODO. test iobject

// test attr
namespace skr::attr::test_rttr
{
sreflect_struct("guid": "37aad92c-09f5-4e18-96ab-eea67094ea42")
AttrA : public skr::rttr::IAttribute {
    SKR_GENERATE_BODY()
    inline AttrA() = default;
    inline AttrA(String content)
        : content(std::move(content))
    {
    }

    String content = {};
};
} // namespace skr::attr::test_rttr

// basic test
namespace test_rttr
{
sreflect_struct("guid": "c7925b0a-e4e7-4d67-b252-62b82a8da018")
BasicBaseA {
};

sreflect_struct("guid": "d716b7bb-dd26-4eb3-b7a9-cdcc1e8994bc")
BasicBaseB {
};

sreflect_struct("guid": "4619687b-71f2-4db6-b3b7-b25fcd63d3e0")
BasicBaseAB : public BasicBaseA,
              public BasicBaseB {
};

sreflect_struct(
    "guid": "65300b18-d043-41ad-a8e4-74c49d97f0d9",
    "rttr": "full",
    "rttr::flags": ["ScriptVisible", "ScriptNewable"],
    "rttr::attrs": ["test_rttr::AttrA{u8\"fuck_u\"}"]
)
BasicRecord : public BasicBaseAB {
    // fields
    sattr("rttr::flags": ["ScriptVisible"])
    sattr("rttr::attrs+": "test_rttr::AttrA{u8\"shit_field\"}")
    int32_t field_int32;
    bool    field_bool;
    float   field_float;
    double  field_double;

    // methods
    sattr("rttr::flags": ["ScriptVisible"])
    sattr("rttr::attrs+": "test_rttr::AttrA{u8\"shit_method\"}")
    void    method_a() {}
    int32_t method_b(float param_a, double param_b) { return 0; }

    // static fields
    sattr("rttr::flags": ["ScriptVisible"])
    sattr("rttr::attrs+": "test_rttr::AttrA{u8\"shit_static_field\"}")
    static int32_t static_field_int32;
    static bool    static_field_bool;
    static float   static_field_float;
    static double  static_field_double;

    // static methods
    sattr("rttr::flags": ["ScriptVisible"])
    sattr("rttr::attrs+": "test_rttr::AttrA{u8\"shit_static_method\"}")
    static void    static_method_a() {}
    static int32_t static_method_b(float param_a, double param_b) { return 0; }
};

sreflect_enum_class(
    "guid": "b5491944-8c97-406d-8281-7b01649751fe",
    "rttr::flags+": "ScriptVisible",
    "rttr::flags+": "Flag",
    "rttr::attrs+": "test_rttr::AttrA{}"
)
BasicEnumClass
{
    A sattr("rttr::flags": ["ScriptVisible"]) sattr("rttr::attrs+": "test_rttr::AttrA{}"),
    B,
    C
};

sreflect_enum(
    "guid": "3a8b2caf-4dba-47cb-a175-8f02e8a90863",
    "rttr::flags": ["ScriptVisible", "Flag"],
    "rttr::attrs+": "test_rttr::AttrA{}"
)
BasicEnum
{
    BasicEnum_A sattr("rttr::flags": ["ScriptVisible"])  sattr("rttr::attrs+": "test_rttr::AttrA{}"),
    BasicEnum_B,
    BasicEnum_C
};

sreflect_struct("guid": "1fe7b9e0-4e08-40ff-a74d-ec8ed0c5a67b", "rttr": false)
DisableRTTR {
};

sreflect_struct(
    "guid": "09a6bdff-0072-4070-840a-5c78c22e84ae",
    "rttr": "full"
)
DisableMemberRTTR {
    // fields
    float field_a;

    sattr("rttr": false)
    float field_b;

    // methods
    void method_a() {}

    sattr("rttr": false)
    void method_b() {}

    // static fields
    static float static_field_a;

    sattr("rttr": false)
    static float static_field_b;

    // static methods
    static void static_method_a() {}

    sattr("rttr": false)
    static void static_method_b() {}
};

} // namespace test_rttr

// test reflect bases and exclude bases
namespace test_rttr
{
struct BadBaseA {
};
struct BadBaseB {
};

sreflect_struct(
    "guid": "735764d0-8aa9-4dbd-8547-09106aab629e",
    "rttr::reflect_bases": false
)
ReflectBases : public BadBaseA,
               public BadBaseB {
};

sreflect_struct(
    "guid": "60dbfdf9-1b48-447a-a821-2b19ef2e47b4",
    "rttr::exclude_bases": ["test_rttr::BadBaseA", "test_rttr::BadBaseB"]
)
ExcludeBases : public BadBaseA,
               public BadBaseB,
               public BasicBaseAB {
};

} // namespace test_rttr