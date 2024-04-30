#include "SkrTestFramework/framework.hpp"
#include "SkrRTTR/export/record_builder.hpp"
#include "SkrRTTR/export/enum_builder.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/export/rttr_backend.hpp"

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
    EnumBuilder<ETestEnum, RTTRBackend> enum_builder(&enum_data);
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