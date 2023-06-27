#pragma once
#include "SkrGui/dev/gdi/gdi_types.hpp"
#include "SkrGui/backend/canvas/canvas_types.hpp"

namespace skr::gui
{
using ::skr::gui::Swizzle;

struct SKR_GUI_API IGDIPaint {
    virtual ~IGDIPaint() SKR_NOEXCEPT = default;

    /*
    virtual void radial_gradient(float cx, float cy, float inr, float outr, skr_float4_t icol, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    virtual void box_gradient(float x, float y, float w, float h, float r, float f, skr_float4_t icol, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    virtual void linear_gradient(float sx, float sy, float ex, float ey, skr_float4_t icol, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    */
    virtual void enable_imagespace_coordinate(bool enable) SKR_NOEXCEPT = 0;
    virtual void custom_vertex_color(CustomVertexPainter painter, void* usrdata) SKR_NOEXCEPT = 0;

    virtual void set_pattern(float cx, float cy, float w, float h, float angle, IGDITexture* texture, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    virtual void set_pattern(float cx, float cy, float w, float h, float angle, IGDIMaterial* material, skr_float4_t ocol) SKR_NOEXCEPT = 0;
};

enum class EGDIWinding : uint32_t
{
    CCW = 1,
    CW = 2,
    Count = 2
};

enum class EGDISolidity : uint32_t
{
    Solid = 1,
    Hole = 2,
    Count = 2
};

struct SKR_GUI_API IGDIElement {
    friend struct IGDIRenderer;
    virtual ~IGDIElement() SKR_NOEXCEPT = default;

    virtual void begin_frame(float devicePixelRatio) = 0;
    virtual void begin_path() = 0;
    virtual void close_path() = 0;
    virtual void arc(float cx, float cy, float r, float a0, float a1, EGDIWinding dir) = 0;
    virtual void rect(float x, float y, float w, float h) = 0;
    virtual void circle(float cx, float cy, float r) = 0;
    virtual void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) = 0;

    virtual void translate(float x, float y) = 0;
    virtual void rotate(float r) = 0;
    virtual void move_to(float x, float y) = 0;
    virtual void line_to(float x, float y) = 0;

    virtual void stroke_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) = 0;
    virtual void stroke_color(float r, float g, float b, float a) = 0;
    virtual void stroke_paint(IGDIPaint* paint) = 0;
    virtual void stroke_width(float size) = 0;
    virtual void stroke() = 0;

    virtual void path_winding(EGDISolidity dir) = 0;

    virtual void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) = 0;
    virtual void fill_color(float r, float g, float b, float a) = 0;
    virtual void fill_paint(IGDIPaint* paint) = 0;
    virtual void fill() = 0;
    virtual void fill_no_aa() = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    // None-VG APIs

    virtual void    set_z(int32_t z) = 0;
    virtual int32_t get_z() const = 0;
    virtual void    set_texture_swizzle(Swizzle swizzle) = 0;
};
} // namespace skr::gui