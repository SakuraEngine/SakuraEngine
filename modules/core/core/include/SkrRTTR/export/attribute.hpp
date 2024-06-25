#pragma once
#include <cstdint>
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
enum class EAttributeTargetFlag : uint16_t
{
    None = 0,

    // function
    Function = 1 << 0,
    Param    = 1 << 1,

    // enum
    Enum      = 1 << 2,
    EnumValue = 1 << 3,

    // record
    Record       = 1 << 4,
    Field        = 1 << 5,
    StaticField  = 1 << 6,
    Method       = 1 << 7,
    StaticMethod = 1 << 8,
    ExternMethod = 1 << 9,
};
struct IAttribute {
    virtual ~IAttribute() = default;
};
} // namespace skr::rttr

SKR_RTTR_TYPE(skr::rttr::IAttribute, "4031a792-6f2c-48ed-90e9-9342fc87809e")