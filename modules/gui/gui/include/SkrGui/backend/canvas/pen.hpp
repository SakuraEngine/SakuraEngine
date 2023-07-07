#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/backend/canvas/canvas_types.hpp"

// pen base
namespace skr::gui
{
enum class EPenType : uint8_t
{
    Fill,
    Stroke,
};

struct Pen {
    inline EPenType type() const SKR_NOEXCEPT { return _type; }

    auto& as_fill() SKR_NOEXCEPT;
    auto& as_fill() const SKR_NOEXCEPT;
    auto& as_stroke() SKR_NOEXCEPT;
    auto& as_stroke() const SKR_NOEXCEPT;

protected:
    inline Pen(EPenType type) SKR_NOEXCEPT
        : _type(type)
    {
    }
    EPenType _type;
};
} // namespace skr::gui

// fill pen
namespace skr::gui
{
struct FillPen : public Pen {
    inline FillPen() SKR_NOEXCEPT
        : Pen(EPenType::Fill)
    {
    }

    // params
    bool _anti_alias = true;

    // builder
    inline FillPen& anti_alias(bool anti_alias) SKR_NOEXCEPT
    {
        _anti_alias = anti_alias;
        return *this;
    }
};
} // namespace skr::gui

// stroke pen
namespace skr::gui
{
enum class EStrokeCap : uint8_t
{
    Butt,
    Round,
    Square,
};

enum class EStrokeJoin : uint8_t
{
    Miter,
    Round,
    Bevel,
};

struct StrokePen : public Pen {
    inline StrokePen() SKR_NOEXCEPT
        : Pen(EPenType::Stroke)
    {
    }

    // params
    float       _width       = 1.0f;
    float       _miter_limit = 10.0f;
    EStrokeCap  _cap         = EStrokeCap::Butt;
    EStrokeJoin _join        = EStrokeJoin::Miter;
    bool        _anti_alias  = true;

    // builder
    inline StrokePen& anti_alias(bool anti_alias) SKR_NOEXCEPT
    {
        _anti_alias = anti_alias;
        return *this;
    }
    inline StrokePen& width(float width) SKR_NOEXCEPT
    {
        _width = width;
        return *this;
    }
    inline StrokePen& cap(EStrokeCap cap) SKR_NOEXCEPT
    {
        _cap = cap;
        return *this;
    }
    inline StrokePen& join(EStrokeJoin join) SKR_NOEXCEPT
    {
        _join = join;
        return *this;
    }
    inline StrokePen& miter_limit(float miter_limit) SKR_NOEXCEPT
    {
        _miter_limit = miter_limit;
        return *this;
    }
};
} // namespace skr::gui

// cast
namespace skr::gui
{
inline auto& Pen::as_fill() SKR_NOEXCEPT
{
    return static_cast<FillPen&>(*this);
}
inline auto& Pen::as_fill() const SKR_NOEXCEPT
{
    return static_cast<const FillPen&>(*this);
}
inline auto& Pen::as_stroke() SKR_NOEXCEPT
{
    return static_cast<StrokePen&>(*this);
}
inline auto& Pen::as_stroke() const SKR_NOEXCEPT
{
    return static_cast<const StrokePen&>(*this);
}
} // namespace skr::gui