#include "SkrTestFramework/framework.hpp"
#include "SkrRTTR/export/export_builder.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrRTTR/export/extern_methods.hpp"

struct TestBase {
};

struct ITestInterface {
};

struct TestDerived : TestBase, virtual ITestInterface {
    TestDerived() {}
    TestDerived(int a) { printf("B(%d)", a); }
    TestDerived(int a, float b) { printf("B(%d, %f)", a, b); }

    void test() {}
    void test(int a, float b) {}

    int32_t i32;
    int64_t i64;

    static void static_test() {}
    static void static_test(int a, float b) {}

    static int32_t static_i32;
    static int64_t static_i64;
};

int32_t TestDerived::static_i32 = 114;
int64_t TestDerived::static_i64 = 514;

void TestExternMethod(TestDerived& test)
{
    printf("TestExternMethod");
}

bool operator==(const TestDerived& lhs, const TestDerived& rhs)
{
    return lhs.i32 == rhs.i32 && lhs.i64 == rhs.i64;
}

SKR_RTTR_TYPE(TestBase, "03ae55ec-5cab-4b57-8c7c-fabddb9c8e87")
SKR_RTTR_TYPE(ITestInterface, "ceb4726c-e321-4dd8-9d18-ddba2019d0f2")
SKR_RTTR_TYPE(TestDerived, "ee9389fe-6152-4540-904b-8424078c963c")

TEST_CASE("test rttr export record")
{
    using namespace skr::rttr;

    // clang-format off
    RecordData                    record_data;
    RecordBuilder<TestDerived> record_builder(&record_data);
    record_builder.basic_info();
    // bases
    record_builder.bases<TestBase, ITestInterface>();
    
    // ctor
    record_builder.ctor<>();
    {
        auto ctor_builder = record_builder.ctor<int>();
        ctor_builder.param_at(0)
            .name(u8"a");
    }
    {
        auto ctor_builder = record_builder.ctor<int, float>();
        ctor_builder.param_at(0)
            .name(u8"a");
        ctor_builder.param_at(1)
            .name(u8"b");
    }

    // methods
    {
        auto method_builder = record_builder.method<void (TestDerived::*)(), &TestDerived::test>(u8"test");
    }
    {
        auto method_builder = record_builder.method<void (TestDerived::*)(int, float), &TestDerived::test>(u8"test");
        method_builder.param_at(0)
            .name(u8"a");
        method_builder.param_at(1)
            .name(u8"b");
    }

    // fields
    record_builder.field<&TestDerived::i32>(u8"i32");
    record_builder.field<&TestDerived::i64>(u8"i64");

    // static methods
    {
        auto static_method_builder = record_builder.static_method<void (*)(), &TestDerived::static_test>(u8"static_test");
    }
    {
        auto static_method_builder = record_builder.static_method<void (*)(int, float), &TestDerived::static_test>(u8"static_test");
        static_method_builder.param_at(0)
            .name(u8"a");
        static_method_builder.param_at(1)
            .name(u8"b");
    }

    // static fields
    record_builder.static_field<&TestDerived::static_i32>(u8"static_i32");
    record_builder.static_field<&TestDerived::static_i64>(u8"static_i64");

    // extern methods
    record_builder.extern_method<&TestExternMethod>(u8"TestExternMethod");
    record_builder.extern_method<+[](const TestDerived& lhs, const TestDerived& rhs){ return lhs == rhs; }>(CPPExternMethods::Eq);
    // clang-format on
}