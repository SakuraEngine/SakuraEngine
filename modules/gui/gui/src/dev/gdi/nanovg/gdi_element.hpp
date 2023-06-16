#pragma once
#include "dev/gdi/private/gdi_element.hpp"
#include <nanovg.h>

namespace skr::gdi
{
struct GDIPaintNVG : public GDIPaintPrivate {
    void set_pattern(float cx, float cy, float w, float h, float angle, IGDITexture* texture, skr_float4_t ocol) SKR_NOEXCEPT final;
    void set_pattern(float cx, float cy, float w, float h, float angle, IGDIMaterial* material, skr_float4_t ocol) SKR_NOEXCEPT final;

    void enable_imagespace_coordinate(bool enable) SKR_NOEXCEPT final;
    void custom_vertex_color(CustomVertexPainter painter, void* usrdata) SKR_NOEXCEPT final;

    NVGpaint            nvg_paint = {};
    skr_float4_t        ocol = {};
    CustomVertexPainter custom_painter = nullptr;
    void*               custom_painter_data = nullptr;
    enum CoordinateMethod
    {
        None,
        NVG,
        ImageSpace,
    } coordinate_method_override = None;
};

struct GDIElementNVG : public GDIElementPrivate {
    inline GDIElementNVG()
        : nvg(nullptr)
    {
    }

    void begin_frame(float devicePixelRatio) final;
    void begin_path() final;
    void close_path() final;
    void arc(float cx, float cy, float r, float a0, float a1, EGDIWinding dir) final;
    void rect(float x, float y, float w, float h) final;
    void circle(float cx, float cy, float r) final;
    void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) final;

    void translate(float x, float y) final;
    void rotate(float r) final;
    void move_to(float x, float y) final;
    void line_to(float x, float y) final;

    void stroke_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) final;
    void stroke_color(float r, float g, float b, float a) final;
    void stroke_paint(IGDIPaint* paint) final;
    void stroke_width(float size) final;
    void stroke() final;

    void path_winding(EGDISolidity dir) final;

    void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) final;
    void fill_color(float r, float g, float b, float a) final;
    void fill_paint(IGDIPaint* paint) final;
    void fill() final;
    void fill_no_aa() final;

    void restore() final;
    void save() final;

    NVGcontext*         nvg = nullptr;
    struct GDIPaintNVG* gdi_paint = nullptr;
};
} // namespace skr::gdi