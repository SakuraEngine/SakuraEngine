#include "SkrTestFramework/framework.hpp"
#include "SkrRTTR/export/export_builder.hpp"
#include "SkrRTTR/rttr_traits.hpp"

enum class ETestEnum
{
    A,
    B,
    C,
    D,
    E,
    F
};
SKR_RTTR_TYPE(ETestEnum, "ec8a3592-a1d7-445f-94a4-6087bd27e006")

TEST_CASE("test rttr export")
{
    using namespace skr::rttr;

    // clang-format off
    EnumData enum_data;
    EnumBuilder<ETestEnum> enum_builder(&enum_data);
    enum_builder.basic_info();
    enum_builder.item(u8"A", ETestEnum::A);
    enum_builder.item(u8"B", ETestEnum::B);
    enum_builder.item(u8"C", ETestEnum::C);
    enum_builder.item(u8"D", ETestEnum::D);
    enum_builder.item(u8"E", ETestEnum::E);
    enum_builder.item(u8"F", ETestEnum::F);
    // clang-format on
}