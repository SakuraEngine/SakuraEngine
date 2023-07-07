#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/backend/canvas/canvas_types.hpp"

// brush base
namespace skr::gui
{
struct ISurface;

enum class EBrushType : uint8_t
{
    Color,
    Surface,
    SurfaceNine,
};

using CustomVertexFuncRef = FunctionRef<void(PaintVertex&)>;

struct ColorBrush;
struct SurfaceBrush;
struct SurfaceNineBrush;

struct Brush {
    inline EBrushType type() const SKR_NOEXCEPT { return _type; }

    auto& as_color() SKR_NOEXCEPT;
    auto& as_color() const SKR_NOEXCEPT;
    auto& as_surface() SKR_NOEXCEPT;
    auto& as_surface() const SKR_NOEXCEPT;
    auto& as_surface_nine() SKR_NOEXCEPT;
    auto& as_surface_nine() const SKR_NOEXCEPT;

    Color _color = { 1, 1, 1, 1 };

protected:
    inline Brush(EBrushType type) SKR_NOEXCEPT
        : _type(type)
    {
    }
    inline Brush(EBrushType type, const Color& color) SKR_NOEXCEPT
        : _color(color),
          _type(type)
    {
    }
    EBrushType _type;
};
} // namespace skr::gui

// color brush
namespace skr::gui
{
struct ColorBrush : public Brush {
    inline ColorBrush() SKR_NOEXCEPT
        : Brush(EBrushType::Color)
    {
    }
    inline ColorBrush(const Color& color) SKR_NOEXCEPT
        : Brush(EBrushType::Color, color)
    {
    }

    // params
    CustomVertexFuncRef _custom = nullptr;

    // builder
    inline ColorBrush& custom(CustomVertexFuncRef custom) SKR_NOEXCEPT
    {
        _custom = custom;
        return *this;
    }
};
} // namespace skr::gui

// surface brush
namespace skr::gui
{
struct SurfaceBrush : public Brush {
    inline SurfaceBrush(ISurface* surface) SKR_NOEXCEPT
        : Brush(EBrushType::Surface),
          _surface(surface)
    {
    }

    // params
    ISurface*           _surface  = nullptr; // TODO. use ISurface
    Rectf               _uv_rect  = {};
    float               _rotation = 0.0f; // in degree
    Swizzle             _swizzle  = {};
    CustomVertexFuncRef _custom   = nullptr;

    // builder
    inline SurfaceBrush& surface(ISurface* surface) SKR_NOEXCEPT
    {
        _surface = surface;
        return *this;
    }
    inline SurfaceBrush& color(Color color) SKR_NOEXCEPT
    {
        _color = color;
        return *this;
    }
    inline SurfaceBrush& uv_rect(Rectf uv_rect) SKR_NOEXCEPT
    {
        _uv_rect = uv_rect;
        return *this;
    }
    inline SurfaceBrush& rotation(float rotation) SKR_NOEXCEPT
    {
        _rotation = rotation;
        return *this;
    }
    inline SurfaceBrush& swizzle(Swizzle swizzle) SKR_NOEXCEPT
    {
        _swizzle = swizzle;
        return *this;
    }
    inline SurfaceBrush& custom(CustomVertexFuncRef custom) SKR_NOEXCEPT
    {
        _custom = custom;
        return *this;
    }
};
} // namespace skr::gui

// surface nine brush
namespace skr::gui
{
struct SurfaceNineBrush : public Brush {
    inline SurfaceNineBrush(ISurface* surface) SKR_NOEXCEPT
        : Brush(EBrushType::SurfaceNine),
          _surface(surface)
    {
    }

    // params
    ISurface*           _surface    = nullptr; // TODO. use ISurface
    Rectf               _uv_rect    = {};
    Rectf               _inner_rect = {};
    float               _rotation   = 0.0f; // in degree
    Swizzle             _swizzle    = {};
    CustomVertexFuncRef _custom     = nullptr;

    // builder
    inline SurfaceNineBrush& surface(ISurface* surface) SKR_NOEXCEPT
    {
        _surface = surface;
        return *this;
    }
    inline SurfaceNineBrush& color(Color color) SKR_NOEXCEPT
    {
        _color = color;
        return *this;
    }
    inline SurfaceNineBrush& uv_rect(Rectf uv_rect) SKR_NOEXCEPT
    {
        _uv_rect = uv_rect;
        return *this;
    }
    inline SurfaceNineBrush& inner_rect(Rectf inner_rect) SKR_NOEXCEPT
    {
        _inner_rect = inner_rect;
        return *this;
    }
    inline SurfaceNineBrush& rotation(float rotation) SKR_NOEXCEPT
    {
        _rotation = rotation;
        return *this;
    }
    inline SurfaceNineBrush& swizzle(Swizzle swizzle) SKR_NOEXCEPT
    {
        _swizzle = swizzle;
        return *this;
    }
    inline SurfaceNineBrush& custom(CustomVertexFuncRef custom) SKR_NOEXCEPT
    {
        _custom = custom;
        return *this;
    }
};
} // namespace skr::gui

// cast
namespace skr::gui
{
inline auto& Brush::as_color() SKR_NOEXCEPT { return static_cast<ColorBrush&>(*this); }
inline auto& Brush::as_color() const SKR_NOEXCEPT { return static_cast<const ColorBrush&>(*this); }
inline auto& Brush::as_surface() SKR_NOEXCEPT { return static_cast<SurfaceBrush&>(*this); }
inline auto& Brush::as_surface() const SKR_NOEXCEPT { return static_cast<const SurfaceBrush&>(*this); }
inline auto& Brush::as_surface_nine() SKR_NOEXCEPT { return static_cast<SurfaceNineBrush&>(*this); }
inline auto& Brush::as_surface_nine() const SKR_NOEXCEPT { return static_cast<const SurfaceNineBrush&>(*this); }
} // namespace skr::gui