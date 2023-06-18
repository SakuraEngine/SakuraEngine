#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/backend/paint_params.hpp"
#include "SkrGui/backend/brush.hpp"

// TODO. remove gdi
#include "SkrGui/dev/gdi/gdi.hpp"

namespace skr::gui
{
struct CanvasPaintScope;
struct CanvasPathFillScope;
struct CanvasPathStrokeScope;
struct CanvasStateScope;
struct Brush;

struct SKR_GUI_API ICanvas final {
    ICanvas(IGDIDevice* device) SKR_NOEXCEPT;
    ~ICanvas() SKR_NOEXCEPT;

    //==> paint scope
    // pixel_ratio = frame_buffer_pixel_size / logical_pixel_size
    // gui 工作在逻辑像素上，逻辑像素的定义由 APP 决定
    // 当逻辑像素被映射到 frame buffer 上时，需要考虑位图缩放以及抗锯齿问题，所以，pixel_ratio 影响如下：
    // 1. 择取合适分辨率的图片
    // 2. 择取合适的 font 字号
    // 3. 控制绘制图元的抗锯齿
    // TODO. remove default value
    void             paint_begin(float pixel_ratio = 1.0f) SKR_NOEXCEPT;
    void             paint_end() SKR_NOEXCEPT;
    CanvasPaintScope paint_scope(float pixel_ratio = 1.0f) SKR_NOEXCEPT;

    //==> states stack
    void             state_save() SKR_NOEXCEPT;
    void             state_restore() SKR_NOEXCEPT;
    CanvasStateScope state_scope() SKR_NOEXCEPT;

    //==> vg states
    void state_reset() SKR_NOEXCEPT;
    void state_stroke_cap(EStrokeCap cap) SKR_NOEXCEPT;      // DEFAULT: Butt
    void state_stroke_join(EStrokeJoin join) SKR_NOEXCEPT;   // DEFAULT: Miter
    void state_stroke_width(float width) SKR_NOEXCEPT;       // DEFAULT: 1.0f
    void state_stroke_miter_limit(float limit) SKR_NOEXCEPT; // DEFAULT: 10.0f
    void state_anti_alias(bool enable) SKR_NOEXCEPT;         // DEFAULT: true

    //==> transform
    void state_transform_reset() SKR_NOEXCEPT;                   // DEFAULT: identity
    void state_translate(Offset offset) SKR_NOEXCEPT;            // APPLY translate
    void state_rotate(float degree) SKR_NOEXCEPT;                // APPLY rotate
    void state_scale(float scale_x, float scale_y) SKR_NOEXCEPT; // APPLY scale
    void state_skew_x(float skew) SKR_NOEXCEPT;                  // APPLY skew
    void state_skew_y(float skew) SKR_NOEXCEPT;                  // APPLY skew

    //==> path
    void                  path_begin() SKR_NOEXCEPT;                    // begin a new path
    void                  path_fill(const Brush& brush) SKR_NOEXCEPT;   // end a path and build vertices for fill
    void                  path_stroke(const Brush& brush) SKR_NOEXCEPT; // end a path and build vertices for stroke
    CanvasPathFillScope   path_fill_scope(const Brush& brush) SKR_NOEXCEPT;
    CanvasPathStrokeScope path_stroke_scope(const Brush& brush) SKR_NOEXCEPT;

    //==> custom path
    void path_move_to(Offset to) SKR_NOEXCEPT;                                                // NEW sub-path from pos
    void path_line_to(Offset to) SKR_NOEXCEPT;                                                // ADD line for CURRENT sub-path
    void path_quad_to(Offset to, Offset control_point) SKR_NOEXCEPT;                          // ADD quadratic bezier for CURRENT sub-path
    void path_cubic_to(Offset to, Offset control_point1, Offset control_point2) SKR_NOEXCEPT; // ADD cubic bezier for CURRENT sub-path
    void path_arc_to(Offset to, Offset control_point, float radius) SKR_NOEXCEPT;             // ADD arc for CURRENT sub-path
    void path_close() SKR_NOEXCEPT;                                                           // CLOSE CURRENT sub-path WITH a line segment

    //==> simple shape path
    void path_arc(Offset center, float radius, float start_degree, float end_degree) SKR_NOEXCEPT; // ADD CLOSED arc sub-path, arc is in CW
    void path_rect(Rect rect) SKR_NOEXCEPT;                                                        // ADD CLOSED rect sub-path
    void path_circle(Offset center, float radius) SKR_NOEXCEPT;                                    // ADD CLOSED circle sub-path
    void path_ellipse(Offset center, float radius_x, float radius_y) SKR_NOEXCEPT;                 // ADD CLOSED ellipse sub-path

    // TODO. custom vertices draw
    // TODO. draw round rect
    // TODO. draw IParagraph
    // TODO. curve quality

private:
    // gdi
    IGDIDevice*         _gdi_device;
    IGDICanvas*         _gdi_canvas;
    IGDIElement*        _current_gdi_element;
    Array<IGDIElement*> _gdi_elements;

    // state & validate
    bool _is_in_paint_scope;
    bool _is_in_path_scope;
};

// scopes
struct CanvasPaintScope {
    inline CanvasPaintScope(ICanvas* canvas, float pixel_ratio) SKR_NOEXCEPT
        : _canvas(canvas)
    {
        _canvas->paint_begin(pixel_ratio);
    }
    inline ~CanvasPaintScope() SKR_NOEXCEPT
    {
        _canvas->paint_end();
    }

private:
    ICanvas* _canvas;
};
struct CanvasPathFillScope {
    inline CanvasPathFillScope(ICanvas* canvas, const Brush& painter) SKR_NOEXCEPT
        : _canvas(canvas),
          _painter(painter)
    {
        _canvas->path_begin();
    }
    inline ~CanvasPathFillScope() SKR_NOEXCEPT
    {
        _canvas->path_fill(_painter);
    }

private:
    ICanvas*     _canvas;
    const Brush& _painter;
};
struct CanvasPathStrokeScope {
    inline CanvasPathStrokeScope(ICanvas* canvas, const Brush& painter) SKR_NOEXCEPT
        : _canvas(canvas),
          _painter(painter)
    {
        _canvas->path_begin();
    }
    inline ~CanvasPathStrokeScope() SKR_NOEXCEPT
    {
        _canvas->path_stroke(_painter);
    }

private:
    ICanvas*     _canvas;
    const Brush& _painter;
};
struct CanvasStateScope {
    inline CanvasStateScope(ICanvas* canvas) SKR_NOEXCEPT
        : _canvas(canvas)
    {
        _canvas->state_save();
    }
    inline ~CanvasStateScope() SKR_NOEXCEPT
    {
        _canvas->state_restore();
    }

private:
    ICanvas* _canvas;
};

inline CanvasPaintScope      ICanvas::paint_scope(float pixel_ratio) SKR_NOEXCEPT { return { this, pixel_ratio }; }
inline CanvasStateScope      ICanvas::state_scope() SKR_NOEXCEPT { return { this }; }
inline CanvasPathFillScope   ICanvas::path_fill_scope(const Brush& brush) SKR_NOEXCEPT { return { this, brush }; }
inline CanvasPathStrokeScope ICanvas::path_stroke_scope(const Brush& brush) SKR_NOEXCEPT { return { this, brush }; }

} // namespace skr::gui
