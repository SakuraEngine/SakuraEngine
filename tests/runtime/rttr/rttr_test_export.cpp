#include "SkrTestFramework/framework.hpp"
#include "SkrRT/rttr/export/record_builder.hpp"
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
    void test(int, float) {}

    int32_t i32;
    int64_t i64;

    static void static_test() {}
    static void static_test(int, float) {}

    static int32_t static_i32;
    static int64_t static_i64;
};

struct TestBackend {
};

SKR_RTTR_TYPE(A, "03ae55ec-5cab-4b57-8c7c-fabddb9c8e87")
SKR_RTTR_TYPE(ITest, "ceb4726c-e321-4dd8-9d18-ddba2019d0f2")
SKR_RTTR_TYPE(B, "ee9389fe-6152-4540-904b-8424078c963c")

TEST_CASE("test rttr export")
{
    using namespace skr::rttr;

    RecordData                    test;
    RecordBuilder<B, TestBackend> builder(&test);
    builder
    // basic
    .base<A>()
    .implement<ITest>()
    // ctor
    .ctor<>()
    .ctor<int>()
    .ctor<int, float>()
    // method
    .method<void (B::*)(), &B::test>(u8"test")
    .method<void (B::*)(int, float), &B::test>(u8"test")
    // field
    .field<&B::i32>(u8"i32")
    .field<&B::i64>(u8"i64")
    // static method
    .static_method<void (*)(), &B::static_test>(u8"static_test")
    .static_method<void (*)(int, float), &B::static_test>(u8"static_test")
    // static field
    .static_field<&B::static_i32>(u8"static_i32")
    .static_field<&B::static_i64>(u8"static_i64");
}