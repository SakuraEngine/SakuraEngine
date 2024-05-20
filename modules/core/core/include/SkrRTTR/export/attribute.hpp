#pragma once
#include <cstdint>

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