#include "dev/gdi/nanovg/gdi_element.hpp"

namespace skr::gui
{

void GDIPaintNVG::set_pattern(float cx, float cy, float w, float h, float angle, IGDITexture* texture, skr_float4_t ocol) SKR_NOEXCEPT
{
    NVGcolor color = nvgRGBAf(ocol.x, ocol.y, ocol.z, ocol.w);
    nvg_paint = nvgImagePattern(nullptr, cx, cy, w, h, angle, texture, color);
}

void GDIPaintNVG::set_pattern(float cx, float cy, float w, float h, float angle, IGDIMaterial* material, skr_float4_t ocol) SKR_NOEXCEPT
{
    NVGcolor color = nvgRGBAf(ocol.x, ocol.y, ocol.z, ocol.w);
    nvg_paint = nvgMaterialPattern(nullptr, cx, cy, w, h, angle, material, color);
}

void GDIPaintNVG::enable_imagespace_coordinate(bool enable) SKR_NOEXCEPT
{
    coordinate_method_override = enable ? NVG : ImageSpace;
}

void GDIPaintNVG::custom_vertex_color(CustomVertexPainter painter, void* usrdata) SKR_NOEXCEPT
{
    custom_painter = painter;
    custom_painter_data = usrdata;
}

void GDIElementNVG::begin_frame(float devicePixelRatio)
{
    nvgBeginFrame(nvg, devicePixelRatio);
    // TODO: retainer box
    const bool dirty = true;
    if (dirty)
    {
        commands.clear();
        vertices.clear();
        indices.clear();
    }
}

void GDIElementNVG::begin_path()
{
    nvgBeginPath(nvg);
}

void GDIElementNVG::arc(float cx, float cy, float r, float a0, float a1, EGDIWinding dir)
{
    const uint32_t dirIndex = static_cast<uint32_t>(dir) - 1;
    const int32_t  WindingLUT[] = { NVG_CW, NVG_CCW };
    nvgArc(nvg, cx, cy, r, a0, a1, WindingLUT[dirIndex]);
}

void GDIElementNVG::close_path()
{
    nvgClosePath(nvg);
}

void GDIElementNVG::rect(float x, float y, float w, float h)
{
    nvgRect(nvg, x, y, w, h);
}

void GDIElementNVG::circle(float cx, float cy, float r)
{
    nvgCircle(nvg, cx, cy, r);
}

void GDIElementNVG::rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft)
{
    nvgRoundedRectVarying(nvg, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}

void GDIElementNVG::translate(float x, float y)
{
    nvgTranslate(nvg, x, y);
}

void GDIElementNVG::rotate(float r)
{
    nvgRotate(nvg, r);
}

void GDIElementNVG::move_to(float x, float y)
{
    nvgMoveTo(nvg, x, y);
}

void GDIElementNVG::line_to(float x, float y)
{
    nvgLineTo(nvg, x, y);
}

void GDIElementNVG::stroke_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    gdi_paint = nullptr;
    nvgStrokeColor(nvg, nvgRGBA(r, g, b, a));
}

void GDIElementNVG::stroke_color(float r, float g, float b, float a)
{
    gdi_paint = nullptr;
    nvgStrokeColor(nvg, nvgRGBAf(r, g, b, a));
}

void GDIElementNVG::stroke_paint(IGDIPaint* paint)
{
    gdi_paint = static_cast<GDIPaintNVG*>(paint);
    auto nvg_paint = static_cast<GDIPaintNVG*>(paint);
    nvgStrokePaint(nvg, nvg_paint->nvg_paint);
}

void GDIElementNVG::stroke_width(float size)
{
    nvgStrokeWidth(nvg, size);
}

void GDIElementNVG::stroke()
{
    nvgStroke(nvg);
}

void GDIElementNVG::path_winding(EGDISolidity dir)
{
    const uint32_t dirIndex = static_cast<uint32_t>(dir) - 1;
    const int32_t  SolidityLUT[] = { NVG_SOLID, NVG_HOLE };
    nvgPathWinding(nvg, SolidityLUT[dirIndex]);
}

void GDIElementNVG::fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    nvgFillColor(nvg, nvgRGBA(r, g, b, a));
}

void GDIElementNVG::fill_color(float r, float g, float b, float a)
{
    nvgFillColor(nvg, nvgRGBAf(r, g, b, a));
}

void GDIElementNVG::fill_paint(IGDIPaint* paint)
{
    gdi_paint = static_cast<GDIPaintNVG*>(paint);
    auto nvg_paint = static_cast<GDIPaintNVG*>(paint);
    nvgFillPaint(nvg, nvg_paint->nvg_paint);
}

void GDIElementNVG::fill()
{
    nvgFill(nvg);
}

void GDIElementNVG::fill_no_aa()
{
    auto params = nvgInternalParams(nvg);
    params->edgeAntiAlias = false;
    nvgFill(nvg);
    params->edgeAntiAlias = true;
}

void GDIElementNVG::restore()
{
    nvgRestore(nvg);
}

void GDIElementNVG::save()
{
    nvgSave(nvg);
}
} // namespace skr::gui