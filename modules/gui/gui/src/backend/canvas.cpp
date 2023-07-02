#include "SkrGui/backend/canvas/canvas.hpp"
#include <nanovg.h>
#include "misc/make_zeroed.hpp"
#include "math/rtm/rtmx.h"
#include "SkrGui/backend/resource/resource.hpp"

// nvg integration
namespace skr::gui
{
struct _NVGHelper {
    inline static uint32_t ToColor32ABGR(NVGcolor color)
    {
        color.r = std::clamp(color.r, 0.f, 1.f);
        color.g = std::clamp(color.g, 0.f, 1.f);
        color.b = std::clamp(color.b, 0.f, 1.f);
        color.a = std::clamp(color.a, 0.f, 1.f);
        uint8_t r = static_cast<uint8_t>(color.r * 255.f);
        uint8_t g = static_cast<uint8_t>(color.g * 255.f);
        uint8_t b = static_cast<uint8_t>(color.b * 255.f);
        uint8_t a = static_cast<uint8_t>(color.a * 255.f);
        return ((uint32_t)a << 24) + ((uint32_t)b << 16) + ((uint32_t)g << 8) + (uint32_t)r;
    }

    static skr_float2_t nvg__remapUV(skr_float2_t is, skr_float2_t size, const NVGbox& box)
    {
        skr_float2_t result;
        if (box.extend[0] == 0.f || box.extend[1] == 0.f)
        {
            return { is.x / size.x, is.y / size.y };
        }

        float marginLeft;
        float marginRight;
        float marginTop;
        float marginBottom;
        {
            if (is.y < box.margin.bottom)
                marginLeft = box.margin.left + box.radius[3];
            else if (is.y < box.margin.bottom + box.radius[3])
            {
                auto off = box.margin.bottom + box.radius[3] - is.y;
                off = off * off + box.radius[3] * box.radius[3];
                marginLeft = box.margin.left + box.radius[3] - std::sqrt(off);
            }
            else if (is.y > (size.y - box.margin.top))
                marginLeft = box.margin.left + box.radius[0];
            else if (is.y > (size.y - box.margin.top) - box.radius[0])
            {
                auto off = is.y - (size.y - box.margin.top) + box.radius[0];
                off = off * off + box.radius[0] * box.radius[0];
                marginLeft = box.margin.left + box.radius[0] - std::sqrt(off);
            }
            else
                marginLeft = box.margin.left;
            if (is.y < box.margin.bottom)
                marginRight = box.margin.right + box.radius[2];
            else if (is.y < box.margin.bottom + box.radius[2])
            {
                auto off = box.margin.bottom + box.radius[2] - is.y;
                off = off * off + box.radius[2] * box.radius[2];
                marginRight = box.margin.right + box.radius[2] - std::sqrt(off);
            }
            else if (is.y > (size.y - box.margin.top))
                marginRight = box.margin.right + box.radius[1];
            else if (is.y > (size.y - box.margin.top) - box.radius[1])
            {
                auto off = is.y - (size.y - box.margin.top) + box.radius[1];
                off = off * off + box.radius[1] * box.radius[1];
                marginRight = box.margin.right + box.radius[1] - std::sqrt(off);
            }
            else
                marginRight = box.margin.right;
            if (is.x < box.margin.left)
                marginTop = box.margin.top + box.radius[0];
            else if (is.x < box.margin.left + box.radius[0])
            {
                auto off = box.margin.left + box.radius[0] - is.x;
                off = off * off + box.radius[0] * box.radius[0];
                marginTop = box.margin.top + box.radius[0] - std::sqrt(off);
            }
            else if (is.x > (size.x - box.margin.right))
                marginTop = box.margin.right + box.radius[1];
            else if (is.x > (size.x - box.margin.right) - box.radius[1])
            {
                auto off = is.x - (size.x - box.margin.right) + box.radius[1];
                off = off * off + box.radius[1] * box.radius[1];
                marginTop = box.margin.top + box.radius[1] - std::sqrt(off);
            }
            else
                marginTop = box.margin.top;
            if (is.x < box.margin.left)
                marginBottom = box.margin.bottom + box.radius[3];
            else if (is.x < box.margin.left + box.radius[3])
            {
                auto off = box.margin.left + box.radius[3] - is.x;
                off = off * off + box.radius[3] * box.radius[3];
                marginBottom = box.margin.bottom + box.radius[3] - std::sqrt(off);
            }
            else if (is.x > (size.x - box.margin.right))
                marginBottom = box.margin.right + box.radius[2];
            else if (is.x > (size.x - box.margin.right) - box.radius[2])
            {
                auto off = is.x - (size.x - box.margin.right) + box.radius[2];
                off = off * off + box.radius[2] * box.radius[2];
                marginBottom = box.margin.bottom + box.radius[2] - std::sqrt(off);
            }
            else
                marginBottom = box.margin.bottom;
        }

        if (is.x < marginLeft)
        {
            result.x = is.x / box.extend[0];
        }
        else if ((size.x - is.x) < marginRight)
        {
            result.x = 1 - (size.x - is.x) / box.extend[0];
        }
        else
        {
            auto alpha = (is.x - marginLeft) / (size.x - marginLeft - marginRight);
            result.x = (marginLeft / box.extend[0]) * alpha + (1 - marginRight / box.extend[0]) * (1 - alpha);
        }

        if (is.y < marginBottom)
        {
            result.y = is.y / box.extend[1];
        }
        else if ((size.y - is.y) < marginTop)
        {
            result.y = 1 - (size.y - is.y) / box.extend[1];
        }
        else
        {
            auto alpha = (is.y - marginBottom) / (size.y - marginBottom - marginTop);
            result.y = (marginBottom / box.extend[1]) * alpha + (1 - marginTop / box.extend[1]) * (1 - alpha);
        }
        return result;
    }

    static void nvg__xformIdentity(float* t)
    {
        t[0] = 1.f;
        t[1] = 0.f;
        t[2] = 0.f;
        t[3] = 1.f;
        t[4] = 0.f;
        t[5] = 0.f;
    }

    static void nvg__xformInverse(float* inv, float* t)
    {
        double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
        if (det > -1e-6 && det < 1e-6)
        {
            nvg__xformIdentity(t);
            return;
        }
        invdet = 1.0 / det;
        inv[0] = (float)(t[3] * invdet);
        inv[2] = (float)(-t[2] * invdet);
        inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
        inv[1] = (float)(-t[1] * invdet);
        inv[3] = (float)(t[0] * invdet);
        inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
    }

    static skr_float4x4_t nvg__getMatrix(NVGpaint* paint)
    {
        float invxform[6];
        nvg__xformInverse(invxform, paint->xform);
        return { { { invxform[0], invxform[1], 0.f, 0.f },
                   { invxform[2], invxform[3], 0.f, 0.f },
                   { 0.f, 0.f, 1.f, 0.f },
                   { invxform[4], invxform[5], 0.f, 1.f } } };
    }

    static void nvg__renderPath(ICanvas* canvas, const NVGpath& path, NVGpaint* paint, const skr_float4x4_t& transform, float fringe)
    {
        skr_float2_t extend{ paint->extent[0], paint->extent[1] };
        auto&        vertices = canvas->_vertices;
        auto&        indices = canvas->_indices;
        auto         push_vertex = [&](const NVGvertex& nv, uint32_t i, uint32_t nfill) {
            auto        brush = canvas->_tmp_brush;
            PaintVertex v;
            v.clipUV = { 0.f, 0.f };
            v.clipUV2 = { 0.f, 0.f };
            v.position = { nv.x, nv.y, 0.f, 1.f };
            v.aa = { nv.u, fringe };
            const rtm::vector4f pos = rtm::vector_load((const uint8_t*)&v.position);
            const auto          col0 = rtm::vector_set(transform.M[0][0], transform.M[0][1], transform.M[0][2], transform.M[0][3]);
            const auto          col1 = rtm::vector_set(transform.M[1][0], transform.M[1][1], transform.M[1][2], transform.M[1][3]);
            const auto          col2 = rtm::vector_set(transform.M[2][0], transform.M[2][1], transform.M[2][2], transform.M[2][3]);
            const auto          col3 = rtm::vector_set(transform.M[3][0], transform.M[3][1], transform.M[3][2], transform.M[3][3]);
            const auto          trans = rtm::matrix_set(col0, col1, col2, col3);
            v.color = ToColor32ABGR(paint->innerColor);

            if (brush->type() == EBrushType::SurfaceNine)
            {
                auto& brush_surface_nine = brush->as_surface_nine();

                // TODO. use brush param
                auto        imgSpace = rtm::matrix_mul_vector(pos, trans);
                const float imgSpaceX = rtm::vector_get_x(imgSpace);
                const float imgSpaceY = rtm::vector_get_y(imgSpace);
                v.texcoord = nvg__remapUV({ imgSpaceX, imgSpaceY }, extend, paint->box);

                if (brush_surface_nine._custom)
                {
                    brush_surface_nine._custom(v);
                }
            }
            else if (brush->type() == EBrushType::Surface)
            {
                auto& brush_surface = brush->as_surface();

                // TODO. use brush param
                auto        imgSpace = rtm::matrix_mul_vector(pos, trans);
                const float imgSpaceX = rtm::vector_get_x(imgSpace);
                const float imgSpaceY = rtm::vector_get_y(imgSpace);
                v.texcoord = nvg__remapUV({ imgSpaceX, imgSpaceY }, extend, paint->box);

                if (brush_surface._custom)
                {
                    brush_surface._custom(v);
                }
            }
            else
            {
                auto& brush_color = brush->as_color();

                v.texcoord = { (float)i / (float)nfill, nv.u };

                if (brush_color._custom)
                {
                    brush_color._custom(v);
                }
            }

            vertices.push_back(v);
        };
        // auto& path = paths[i];
        if (path.nfill)
        {
            vertices.reserve(vertices.size() + path.nfill);
            indices.reserve(indices.size() + path.nfill * 3);
            const auto start = static_cast<PaintIndex>(vertices.size());
            for (int j = 0; j < path.nfill; ++j)
            {
                push_vertex(path.fill[j], j, path.nfill);
                if (j < path.nfill - 2)
                {
                    const auto id = static_cast<PaintIndex>(vertices.size());
                    indices.push_back(start);
                    indices.push_back(id + 1);
                    indices.push_back(id);
                }
            }
        }
        if (path.nstroke)
        {
            vertices.reserve(vertices.size() + path.nstroke);
            indices.reserve(indices.size() + path.nstroke * 3);
            for (int j = 0; j < path.nstroke; ++j)
            {
                push_vertex(path.stroke[j], j, path.nstroke);
                if (j < path.nstroke - 2)
                {
                    const auto id = static_cast<PaintIndex>(vertices.size() - 1);
                    indices.push_back(id);
                    indices.push_back(id + 1 + (j % 2));
                    indices.push_back(id + 1 + !(j % 2));
                }
            }
        }
    }

    static void nvg__renderFill(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, float fringe,
                                const float* bounds, const NVGpath* paths, int npaths)
    {
        // fast path
        if (npaths == 1 && paths[0].convex)
        {
            // init data
            auto  canvas = (ICanvas*)uptr;
            auto  invTransform = nvg__getMatrix(paint);
            auto& command = canvas->_commands.emplace_back();
            auto  begin = canvas->_indices.size();

            // combine vertices
            for (int i = 0; i < npaths; ++i)
            {
                nvg__renderPath(canvas, paths[i], paint, invTransform, 1.f);
            }

            // combine commands
            if (canvas->_tmp_brush->type() == EBrushType::Surface)
            {
                const auto surface_brush = static_cast<const SurfaceBrush*>(canvas->_tmp_brush);
                command.texture_swizzle = surface_brush->_swizzle;
            }
            else if (canvas->_tmp_brush->type() == EBrushType::SurfaceNine)
            {
                const auto surface_nine_brush = static_cast<const SurfaceBrush*>(canvas->_tmp_brush);
                command.texture_swizzle = surface_nine_brush->_swizzle;
            }

            command.index_begin = static_cast<uint32_t>(begin);
            command.index_count = static_cast<uint32_t>(canvas->_indices.size() - begin);
            command.material = static_cast<IMaterial*>(paint->material);
            command.texture = static_cast<IImage*>(paint->image);
            if (command.texture && command.texture->is_okey())
            {
                command.texture = nullptr;
            }
        }
        // slow path
        else
        {
            SKR_UNREACHABLE_CODE()
        }
    }

    static void nvg__renderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, float fringe,
                                  float strokeWidth, const NVGpath* paths, int npaths)
    {
        // init data
        auto  canvas = (ICanvas*)uptr;
        auto  invTransform = nvg__getMatrix(paint);
        auto& command = canvas->_commands.emplace_back();
        auto  begin = canvas->_indices.size();
        float aa = (fringe * 0.5f + strokeWidth * 0.5f) / fringe;

        // combine vertices & indices
        for (int i = 0; i < npaths; ++i)
        {
            nvg__renderPath(canvas, paths[i], paint, invTransform, aa);
        }

        // combine command
        command.index_begin = static_cast<uint32_t>(begin);
        command.index_count = static_cast<uint32_t>(canvas->_indices.size() - begin);
        command.material = static_cast<IMaterial*>(paint->material);
        command.texture = static_cast<IImage*>(paint->image);
        if (command.texture && command.texture->is_okey())
        {
            command.texture = nullptr;
        }
    }
};
} // namespace skr::gui

namespace skr::gui
{
ICanvas::ICanvas() SKR_NOEXCEPT
{
    auto params = make_zeroed<NVGparams>();
    params.renderFill = _NVGHelper::nvg__renderFill;
    params.renderStroke = _NVGHelper::nvg__renderStroke;
    params.userPtr = this;
    params.edgeAntiAlias = true;
    _nvg = nvgCreateInternal(&params);
}

ICanvas::~ICanvas() SKR_NOEXCEPT
{
    nvgDeleteInternal(_nvg);
}

void ICanvas::clear() SKR_NOEXCEPT
{
    _indices.clear();
    _vertices.clear();
    _commands.clear();
}

//==> paint scope
void ICanvas::paint_begin(float pixel_ratio) SKR_NOEXCEPT
{
    // validate
    if (_is_in_paint_scope)
    {
        SKR_GUI_LOG_ERROR("ICanvas::paint_begin() called without a matching ICanvas::paint_end() call");
    }

    // begin frame
    nvgBeginFrame(_nvg, pixel_ratio);
    _is_in_paint_scope = true;
}
void ICanvas::paint_end() SKR_NOEXCEPT
{
    // validate
    if (!_is_in_paint_scope)
    {
        SKR_GUI_LOG_ERROR("ICanvas::paint_end() called without a matching ICanvas::paint_begin() call");
    }

    // end frame
    _is_in_paint_scope = false;
}

//==> states stack
void ICanvas::state_save() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgSave(_nvg);
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_save() called without a matching ICanvas::state_restore() call");
    }
}
void ICanvas::state_restore() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgRestore(_nvg);
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_restore() called without a matching ICanvas::state_save() call");
    }
}

//==> state
void ICanvas::state_reset() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgReset(_nvg);
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_reset() called outside of a paint scope");
    }
}
void ICanvas::state_translate(Offsetf offset) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgTranslate(_nvg, offset.x, offset.y);
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_translate() called outside of a paint scope");
    }
}
void ICanvas::state_rotate(float degree) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgRotate(_nvg, nvgDegToRad(degree));
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_rotate() called outside of a paint scope");
    }
}
void ICanvas::state_scale(float scale_x, float scale_y) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgScale(_nvg, scale_x, scale_y);
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_scale() called outside of a paint scope");
    }
}
void ICanvas::state_skew_x(float degree) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgSkewX(_nvg, nvgDegToRad(degree));
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_skew_x() called outside of a paint scope");
    }
}
void ICanvas::state_skew_y(float degree) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgSkewY(_nvg, nvgDegToRad(degree));
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_skew_y() called outside of a paint scope");
    }
}

//==> path scope
void ICanvas::path_begin() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        nvgBeginPath(_nvg);
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_begin() called outside of a paint scope");
    }

    _is_in_path_scope = true;
}
void ICanvas::path_end(const Pen& pen, const Brush& brush) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        _tmp_brush = &brush;
        switch (pen.type())
        {
            case EPenType::Fill: {
                nvgFillColor(_nvg, nvgRGBAf(brush._color.r, brush._color.g, brush._color.b, brush._color.a));

                const FillPen& fill_pen = pen.as_fill();
                nvgShapeAntiAlias(_nvg, fill_pen._anti_alias);
                nvgFill(_nvg);
                break;
            }
            case EPenType::Stroke: {
                nvgStrokeColor(_nvg, nvgRGBAf(brush._color.r, brush._color.g, brush._color.b, brush._color.a));

                const StrokePen& stroke_pen = pen.as_stroke();
                nvgStrokeWidth(_nvg, stroke_pen._width);
                nvgMiterLimit(_nvg, stroke_pen._miter_limit);
                nvgLineCap(_nvg, static_cast<int>(stroke_pen._cap));
                nvgLineJoin(_nvg, static_cast<int>(stroke_pen._join));
                // TODO. AA
                nvgStroke(_nvg);
                break;
            }
        }
        _tmp_brush = nullptr;
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_fill() called outside of a paint scope");
    }

    _is_in_path_scope = false;
}

//==> path
void ICanvas::path_move_to(Offsetf to) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgMoveTo(_nvg, to.x, to.y);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_move_to() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_move_to() called outside of a paint scope");
    }
}
void ICanvas::path_line_to(Offsetf to) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgLineTo(_nvg, to.x, to.y);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_line_to() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_line_to() called outside of a paint scope");
    }
}
void ICanvas::path_quad_to(Offsetf to, Offsetf control_point) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgQuadTo(_nvg, control_point.x, control_point.y, to.x, to.y);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_quad_to() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_quad_to() called outside of a paint scope");
    }
}
void ICanvas::path_cubic_to(Offsetf to, Offsetf control_point1, Offsetf control_point2) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgBezierTo(_nvg, control_point1.x, control_point1.y, control_point2.x, control_point2.y, to.x, to.y);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_cubic_to() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_cubic_to() called outside of a paint scope");
    }
}
void ICanvas::path_arc_to(Offsetf to, Offsetf control_point, float radius) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgArcTo(_nvg, control_point.x, control_point.y, to.x, to.y, radius);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_arc_to() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_arc_to() called outside of a paint scope");
    }
}
void ICanvas::path_close() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgClosePath(_nvg);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_close() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_close() called outside of a paint scope");
    }
}

//==> close path
void ICanvas::path_arc(Offsetf center, float radius, float start_degree, float end_degree) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgArc(_nvg, center.x, center.y, radius, nvgDegToRad(start_degree), nvgDegToRad(end_degree), NVG_CW);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_arc() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_arc() called outside of a paint scope");
    }
}
void ICanvas::path_rect(Rectf rect) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgRect(_nvg, rect.left, rect.top, rect.width(), rect.height());
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_rect() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_rect() called outside of a paint scope");
    }
}
void ICanvas::path_circle(Offsetf center, float radius) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgCircle(_nvg, center.x, center.y, radius);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_circle() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_circle() called outside of a paint scope");
    }
}
void ICanvas::path_ellipse(Offsetf center, float radius_x, float radius_y) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            nvgEllipse(_nvg, center.x, center.y, radius_x, radius_y);
        }
        else
        {
            SKR_GUI_LOG_ERROR("ICanvas::path_ellipse() called outside of a path scope");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_ellipse() called outside of a paint scope");
    }
}

//==> draw primitives
void ICanvas::draw_arc(Offsetf center, float radius, float start_degree, float end_degree, const Pen& pen, const Brush& brush) SKR_NOEXCEPT
{
    auto _ = path_scope(pen, brush);
    path_arc(center, radius, start_degree, end_degree);
}
void ICanvas::draw_rect(Rectf rect, const Pen& pen, const Brush& brush) SKR_NOEXCEPT
{
    auto _ = path_scope(pen, brush);
    path_rect(rect);
}
void ICanvas::draw_circle(Offsetf center, float radius, const Pen& pen, const Brush& brush) SKR_NOEXCEPT
{
    auto _ = path_scope(pen, brush);
    path_circle(center, radius);
}
void ICanvas::draw_ellipse(Offsetf center, float radius_x, float radius_y, const Pen& pen, const Brush& brush) SKR_NOEXCEPT
{
    auto _ = path_scope(pen, brush);
    path_ellipse(center, radius_x, radius_y);
}
void ICanvas::draw_image(Rectf rect, const Pen& pen, const Brush& brush) SKR_NOEXCEPT
{
    // TODO. draw image
    draw_rect(rect, pen, brush);
}
void ICanvas::draw_image_nine(Rectf rect, const Pen& pen, const Brush& brush) SKR_NOEXCEPT
{
    // TODO. draw image nine
    draw_rect(rect, pen, brush);
}

} // namespace skr::gui