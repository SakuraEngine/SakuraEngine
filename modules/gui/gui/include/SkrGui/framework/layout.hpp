#pragma once
#include "SkrGui/framework/geometry.hpp"

namespace skr::gui
{
// 文字布局朝向
enum class TextDirection
{
    LTR,
    RTL,
};

// 布局定位使用的基本单位，在实现 Optional 功能的同时
struct PositionalUnit {
    enum Type
    {
        Unset,
        Pixel,
        Percent,
    };

    float value = 0;
    uint8_t type = Type::Unset;

    // construct
    constexpr PositionalUnit() = default;
    constexpr PositionalUnit(float value)
        : value(value)
        , type(Type::Pixel)
    {
    }
    constexpr PositionalUnit(float value, Type type)
        : value(value)
        , type(type)
    {
    }

    // compare
    constexpr explicit operator bool() const { return type != Type::Unset; }
    constexpr bool operator==(std::nullptr_t) const { return type == Type::Unset; }
    constexpr bool operator!=(std::nullptr_t) const { return type != Type::Unset; }
    constexpr bool operator<=>(const PositionalUnit& other) const = default;

    // factory
    static constexpr PositionalUnit null() { return PositionalUnit{}; }
    static constexpr PositionalUnit px(float value) { return PositionalUnit{ value, Type::Pixel }; }
    static constexpr PositionalUnit pct(float value) { return PositionalUnit{ value, Type::Percent }; }

    // calc
    constexpr float get_pixel(float parent_value) const
    {
        if (type == Type::Percent)
        {
            return parent_value * value;
        }
        else
        {
            return value;
        }
    }
};

// float literal
constexpr PositionalUnit operator""_px(long double value) { return PositionalUnit::px(static_cast<float>(value)); }
constexpr PositionalUnit operator""_pct(long double value) { return PositionalUnit::pct(static_cast<float>(value)); }

// int literal
constexpr PositionalUnit operator""_px(unsigned long long value) { return PositionalUnit::px(static_cast<float>(value)); }
constexpr PositionalUnit operator""_pct(unsigned long long value) { return PositionalUnit::pct(static_cast<float>(value)); }

// 布局定位 + 约束
struct Positional {
    // 约束定位 or 锚点定位
    PositionalUnit left = PositionalUnit::null();
    PositionalUnit top = PositionalUnit::null();
    PositionalUnit right = PositionalUnit::null();
    PositionalUnit bottom = PositionalUnit::null();

    // 锚点定位中使用的尺寸约束
    PositionalUnit min_width = PositionalUnit::null();
    PositionalUnit max_width = PositionalUnit::null();
    PositionalUnit min_height = PositionalUnit::null();
    PositionalUnit max_height = PositionalUnit::null();

    // 锚点
    Offset pivot = { 0, 0 };
};

// Box 约束器
struct BoxConstraint {
    float min_width = 0;
    float max_width = std::numeric_limits<float>::infinity();
    float min_height = 0;
    float max_height = std::numeric_limits<float>::infinity();
};

} // namespace skr::gui