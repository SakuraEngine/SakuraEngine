#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/backend/canvas/canvas_types.hpp"
#include "SkrGui/backend/canvas/brush.hpp"
#include "SkrGui/backend/canvas/pen.hpp"

struct NVGcontext;
namespace skr::gui
{
struct CanvasPaintScope;
struct CanvasPathFillScope;
struct CanvasPathStrokeScope;
struct CanvasStateScope;
struct Brush;

struct SKR_GUI_API ICanvas final {
    ICanvas() SKR_NOEXCEPT;
    ~ICanvas() SKR_NOEXCEPT;

    // TODO. move into paint begin
    void clear() SKR_NOEXCEPT;

    //==> paint scope
    // pixel_ratio = frame_buffer_pixel_size / logical_pixel_size
    // gui 工作在逻辑像素上，逻辑像素的定义由 APP 决定
    // 当逻辑像素被映射到 frame buffer 上时，需要考虑位图缩放以及抗锯齿问题，所以，pixel_ratio 影响如下：
    // 1. 择取合适分辨率的图片
    // 2. 择取合适的 font 字号
    // 3. 控制绘制图元的抗锯齿
    // TODO. remove default value
    void paint_begin(float pixel_ratio = 1.0f) SKR_NOEXCEPT;
    void paint_end() SKR_NOEXCEPT;
    auto paint_scope(float pixel_ratio = 1.0f) SKR_NOEXCEPT;

    //==> states stack
    void state_save() SKR_NOEXCEPT;
    void state_restore() SKR_NOEXCEPT;
    auto state_scope() SKR_NOEXCEPT;

    //==> state
    void state_reset() SKR_NOEXCEPT;                             // DEFAULT: identity
    void state_translate(Offsetf offset) SKR_NOEXCEPT;           // APPLY translate
    void state_rotate(float degree) SKR_NOEXCEPT;                // APPLY rotate
    void state_scale(float scale_x, float scale_y) SKR_NOEXCEPT; // APPLY scale
    void state_skew_x(float degree) SKR_NOEXCEPT;                // APPLY skew
    void state_skew_y(float degree) SKR_NOEXCEPT;                // APPLY skew

    //==> path scope
    void path_begin() SKR_NOEXCEPT;                                 // begin a new path
    void path_end(const Pen& pen, const Brush& brush) SKR_NOEXCEPT; // end path and combine vertices
    auto path_scope(const Pen& pen, const Brush& brush) SKR_NOEXCEPT;

    //==> path
    void path_move_to(Offsetf to) SKR_NOEXCEPT;                                                  // NEW sub-path from pos
    void path_line_to(Offsetf to) SKR_NOEXCEPT;                                                  // ADD line for CURRENT sub-path
    void path_quad_to(Offsetf to, Offsetf control_point) SKR_NOEXCEPT;                           // ADD quadratic bezier for CURRENT sub-path
    void path_cubic_to(Offsetf to, Offsetf control_point1, Offsetf control_point2) SKR_NOEXCEPT; // ADD cubic bezier for CURRENT sub-path
    void path_arc_to(Offsetf to, Offsetf control_point, float radius) SKR_NOEXCEPT;              // ADD arc for CURRENT sub-path
    void path_close() SKR_NOEXCEPT;                                                              // CLOSE CURRENT sub-path WITH a line segment

    //==> closed path
    void path_arc(Offsetf center, float radius, float start_degree, float end_degree) SKR_NOEXCEPT; // ADD CLOSED arc sub-path, arc is in CW
    void path_rect(Rectf rect) SKR_NOEXCEPT;                                                        // ADD CLOSED rect sub-path
    void path_circle(Offsetf center, float radius) SKR_NOEXCEPT;                                    // ADD CLOSED circle sub-path
    void path_ellipse(Offsetf center, float radius_x, float radius_y) SKR_NOEXCEPT;                 // ADD CLOSED ellipse sub-path

    //==> draw primitives
    void draw_arc(Offsetf center, float radius, float start_degree, float end_degree, const Pen& pen, const Brush& brush) SKR_NOEXCEPT; // DRAW arc, arc is in CW
    void draw_rect(Rectf rect, const Pen& pen, const Brush& brush) SKR_NOEXCEPT;                                                        // DRAW rect
    void draw_circle(Offsetf center, float radius, const Pen& pen, const Brush& brush) SKR_NOEXCEPT;                                    // DRAW circle
    void draw_ellipse(Offsetf center, float radius_x, float radius_y, const Pen& pen, const Brush& brush) SKR_NOEXCEPT;                 // DRAW ellipse
    void draw_image(Rectf rect, const Pen& pen, const Brush& brush) SKR_NOEXCEPT;                                                       // DRAW image
    void draw_image_nine(Rectf rect, const Pen& pen, const Brush& brush) SKR_NOEXCEPT;                                                  // DRAW image nine

    // TODO. custom vertices draw
    // TODO. draw round rect
    // TODO. draw IParagraph
    // TODO. curve quality

    // getter
    inline Span<const PaintVertex>  vertices() const SKR_NOEXCEPT { return { _vertices.data(), _vertices.size() }; }
    inline Span<const PaintIndex>   indices() const SKR_NOEXCEPT { return { _indices.data(), _indices.size() }; }
    inline Span<const PaintCommand> commands() const SKR_NOEXCEPT { return { _commands.data(), _commands.size() }; }

private:
    friend struct _NVGHelper;

    // nvg
    Array<PaintVertex>  _vertices;
    Array<PaintIndex>   _indices;
    Array<PaintCommand> _commands;
    NVGcontext*         _nvg       = nullptr;
    const Brush*        _tmp_brush = nullptr;

    // state & validate
    bool _is_in_paint_scope = false;
    bool _is_in_path_scope  = false;
};

inline auto ICanvas::paint_scope(float pixel_ratio) SKR_NOEXCEPT
{
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
    return CanvasPaintScope{ this, pixel_ratio };
}
inline auto ICanvas::state_scope() SKR_NOEXCEPT
{
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
    return CanvasStateScope{ this };
}
inline auto ICanvas::path_scope(const Pen& pen, const Brush& brush) SKR_NOEXCEPT
{
    struct CanvasPathFillScope {
        inline CanvasPathFillScope(ICanvas* canvas, const Pen& pen, const Brush& brush) SKR_NOEXCEPT
            : _canvas(canvas),
              _pen(pen),
              _brush(brush)
        {
            _canvas->path_begin();
        }
        inline ~CanvasPathFillScope() SKR_NOEXCEPT
        {
            _canvas->path_end(_pen, _brush);
        }

    private:
        ICanvas*     _canvas;
        const Pen&   _pen;
        const Brush& _brush;
    };
    return CanvasPathFillScope{ this, pen, brush };
}

} // namespace skr::gui
