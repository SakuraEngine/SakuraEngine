#include "SkrTestFramework/framework.hpp"
#include "SkrRT/rttr/export/record_builder.hpp"
#include "SkrRT/rttr/export/enum_builder.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

struct A {
};

struct ITest {
};

struct B : A, ITest {
    B() {}
    B(int a) { printf("B(%d)", a); }
    B(int a, float b) { printf("B(%d, %f)", a, b); }

    void test() {}
    void test(int a, float b) {}

    int32_t i32;
    int64_t i64;

    static void static_test() {}
    static void static_test(int a, float b) {}

    static int32_t static_i32;
    static int64_t static_i64;
};

int32_t B::static_i32 = 114;
int64_t B::static_i64 = 514;

enum class ETestEnum
{
    A,
    B,
    C,
    D,
    E,
    F
};

struct TestBackend {
};

SKR_RTTR_TYPE(A, "03ae55ec-5cab-4b57-8c7c-fabddb9c8e87")
SKR_RTTR_TYPE(ITest, "ceb4726c-e321-4dd8-9d18-ddba2019d0f2")
SKR_RTTR_TYPE(B, "ee9389fe-6152-4540-904b-8424078c963c")
SKR_RTTR_TYPE(ETestEnum, "ec8a3592-a1d7-445f-94a4-6087bd27e006")

TEST_CASE("test rttr export")
{
    using namespace skr::rttr;

    // clang-format off
    RecordData                    record_data;
    RecordBuilder<B, TestBackend> record_builder(&record_data);
    record_builder
        // basic
        .basic_info()
        .bases<A, ITest>()
        // ctor
        .ctor<>()
        .ctor<int>()
            .param(0, u8"a")
        .ctor<int, float>()
            .param(0, u8"a")
            .param(1, u8"b")
        // method
        .method<void (B::*)(), &B::test>(u8"test")
        .method<void (B::*)(int, float), &B::test>(u8"test")
            .param(0, u8"a")
            .param(1, u8"b")
        // field
        .field<&B::i32>(u8"i32")
        .field<&B::i64>(u8"i64")
        // static method
        .static_method<void (*)(), &B::static_test>(u8"static_test")
        .static_method<void (*)(int, float), &B::static_test>(u8"static_test")
            .param(0, u8"a")
            .param(1, u8"b")
        // static field
        .static_field<&B::static_i32>(u8"static_i32")
        .static_field<&B::static_i64>(u8"static_i64");

    EnumData enum_data;
    EnumBuilder<ETestEnum, TestBackend> enum_builder(&enum_data);
    enum_builder
        // basic
        .basic_info()
        // items
        .item(u8"A", ETestEnum::A)
        .item(u8"B", ETestEnum::B)
        .item(u8"C", ETestEnum::C)
        .item(u8"D", ETestEnum::D)
        .item(u8"E", ETestEnum::E)
        .item(u8"F", ETestEnum::F);

    // clang-format on
}