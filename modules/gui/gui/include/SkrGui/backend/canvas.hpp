#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/backend/paint_params.hpp"

// TODO. remove gdi
#include "SkrGui/dev/gdi/gdi.hpp"

namespace skr::gui
{
using ::skr::gdi::IGDICanvas;
using ::skr::gdi::IGDIDevice;
using ::skr::gdi::IGDIElement;
using ::skr::gdi::IGDIPaint;

struct CanvasPaintScope;
struct CanvasPathScope;
struct CanvasStateScope;

struct ColorPaintBuilder;
struct TexturePaintBuilder;
struct MaterialPaintBuilder;

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
    void state_paint_style(EPaintStyle style) SKR_NOEXCEPT;  // DEFAULT: Fill
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

    //==> paint states
    void                 state_paint_reset() SKR_NOEXCEPT;                       // DEFAULT: Color { white }
    ColorPaintBuilder    state_paint_color(Color color) SKR_NOEXCEPT;            // SWITCH paint mode to color
    TexturePaintBuilder  state_paint_texture(ITexture* texture) SKR_NOEXCEPT;    // SWITCH paint mode to texture
    MaterialPaintBuilder state_paint_material(IMaterial* material) SKR_NOEXCEPT; // SWITCH paint mode to material

    //==> path
    void            path_begin() SKR_NOEXCEPT; // begin a new path
    void            path_end() SKR_NOEXCEPT;   // end a path and build vertices by current [fill-states and paint-states]
    CanvasPathScope path_scope() SKR_NOEXCEPT;

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

    //==> helper draw, warper for path
    void draw_arc(Offset center, float radius, float start_degree, float end_degree) SKR_NOEXCEPT;
    void draw_rect(Rect rect) SKR_NOEXCEPT;
    void draw_circle(Offset center, float radius) SKR_NOEXCEPT;
    void draw_ellipse(Offset center, float radius_x, float radius_y) SKR_NOEXCEPT;

    // TODO. custom vertices draw
    // TODO. draw round rect
    // TODO. draw IParagraph

private:
    // paint mode builder
    void _state_paint_color(Color color) SKR_NOEXCEPT;                                       // DEFAULT: white (Color { 1, 1, 1, 1 })
    void _state_paint_uv_rect(Rect uv_rect) SKR_NOEXCEPT;                                    // DEFAULT: empty rect { 0, 0, 0, 0 }
    void _state_paint_uv_rect_nine(Rect center, Rect total) SKR_NOEXCEPT;                    // DEFAULT: empty rect { 0, 0, 0, 0 }
    void _state_paint_blend_mode(BlendMode mode) SKR_NOEXCEPT;                               // DEFAULT: { 1, 1 - SrcAlpha, 1, 1 - SrcAlpha }
    void _state_paint_rotation(float degree) SKR_NOEXCEPT;                                   // DEFAULT: 0.0f
    void _state_paint_texture_swizzle(Swizzle swizzle) SKR_NOEXCEPT;                         // DEFAULT: { R, G, B, A }
    void _state_paint_custom_paint(CustomPaintCallback custom, void* userdata) SKR_NOEXCEPT; // DEFAULT: nullptr

private:
    friend struct ColorPaintBuilder;
    friend struct TexturePaintBuilder;
    friend struct MaterialPaintBuilder;

    struct _State {
        // TODO. move to GDI
        bool anti_alias = true;

        EPaintStyle paint_style = EPaintStyle::Fill;

        EPaintType          paint_type = EPaintType::Color;
        Color               color = { 1, 1, 1, 1 };
        ITexture*           texture = nullptr;
        IMaterial*          material = nullptr;
        Rect                uv_rect = {};
        Rect                uv_rect_nine_total = {};
        BlendMode           blend_mode = {};
        float               degree = 0.0f;
        Swizzle             swizzle = {};
        CustomPaintCallback custom_paint = nullptr;
        void*               custom_paint_userdata = nullptr;
    };

    // gdi
    IGDIDevice*         _gdi_device;
    IGDICanvas*         _gdi_canvas;
    IGDIElement*        _current_gdi_element;
    Array<IGDIElement*> _gdi_elements;

    // state & validate
    Array<_State> _state_stack;
    _State        _current_state;
    bool          _is_in_paint_scope;
    bool          _is_in_path_scope;
};

// paint mode builders
struct ColorPaintBuilder {
    ColorPaintBuilder& custom(CustomPaintCallback callback, void* userdata) SKR_NOEXCEPT
    {
        _canvas->_state_paint_custom_paint(callback, userdata);
        return *this;
    }

private:
    friend struct ICanvas;
    inline ColorPaintBuilder(ICanvas* canvas) SKR_NOEXCEPT
        : _canvas(canvas)
    {
    }
    ColorPaintBuilder(const ColorPaintBuilder&) = delete;
    ColorPaintBuilder(ColorPaintBuilder&&) = delete;
    ColorPaintBuilder& operator=(const ColorPaintBuilder&) = delete;
    ColorPaintBuilder& operator=(ColorPaintBuilder&&) = delete;
    ICanvas*           _canvas;
};
struct TexturePaintBuilder {
    TexturePaintBuilder& color(Color color) SKR_NOEXCEPT
    {
        _canvas->_state_paint_color(color);
        return *this;
    }
    TexturePaintBuilder& uv(Rect uv) SKR_NOEXCEPT
    {
        _canvas->_state_paint_uv_rect(uv);
        return *this;
    }
    TexturePaintBuilder& uv_nine(Rect center, Rect total) SKR_NOEXCEPT
    {
        _canvas->_state_paint_uv_rect_nine(center, total);
        return *this;
    }
    TexturePaintBuilder& rotation(float degree) SKR_NOEXCEPT
    {
        _canvas->_state_paint_rotation(degree);
        return *this;
    }
    TexturePaintBuilder& blend(BlendMode mode) SKR_NOEXCEPT
    {
        _canvas->_state_paint_blend_mode(mode);
        return *this;
    }
    TexturePaintBuilder& swizzle(Swizzle swizzle) SKR_NOEXCEPT
    {
        _canvas->_state_paint_texture_swizzle(swizzle);
        return *this;
    }
    TexturePaintBuilder& custom(CustomPaintCallback callback, void* userdata) SKR_NOEXCEPT
    {
        _canvas->_state_paint_custom_paint(callback, userdata);
        return *this;
    }

private:
    friend struct ICanvas;
    inline TexturePaintBuilder(ICanvas* canvas) SKR_NOEXCEPT
        : _canvas(canvas)
    {
    }
    TexturePaintBuilder(const TexturePaintBuilder&) = delete;
    TexturePaintBuilder(TexturePaintBuilder&&) = delete;
    TexturePaintBuilder& operator=(const TexturePaintBuilder&) = delete;
    TexturePaintBuilder& operator=(TexturePaintBuilder&&) = delete;
    ICanvas*             _canvas;
};
struct MaterialPaintBuilder {
    MaterialPaintBuilder& color(Color color) SKR_NOEXCEPT
    {
        _canvas->_state_paint_color(color);
        return *this;
    }
    MaterialPaintBuilder& uv(Rect uv) SKR_NOEXCEPT
    {
        _canvas->_state_paint_uv_rect(uv);
        return *this;
    }
    MaterialPaintBuilder& uv_nine(Rect center, Rect total) SKR_NOEXCEPT
    {
        _canvas->_state_paint_uv_rect_nine(center, total);
        return *this;
    }
    MaterialPaintBuilder& rotation(float degree) SKR_NOEXCEPT
    {
        _canvas->_state_paint_rotation(degree);
        return *this;
    }
    MaterialPaintBuilder& custom(CustomPaintCallback callback, void* userdata) SKR_NOEXCEPT
    {
        _canvas->_state_paint_custom_paint(callback, userdata);
        return *this;
    }

private:
    friend struct ICanvas;
    inline MaterialPaintBuilder(ICanvas* canvas) SKR_NOEXCEPT
        : _canvas(canvas)
    {
    }
    MaterialPaintBuilder(const MaterialPaintBuilder&) = delete;
    MaterialPaintBuilder(MaterialPaintBuilder&&) = delete;
    MaterialPaintBuilder& operator=(const MaterialPaintBuilder&) = delete;
    MaterialPaintBuilder& operator=(MaterialPaintBuilder&&) = delete;
    ICanvas*              _canvas;
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
struct CanvasPathScope {
    inline CanvasPathScope(ICanvas* canvas) SKR_NOEXCEPT
        : _canvas(canvas)
    {
        _canvas->path_begin();
    }
    inline ~CanvasPathScope() SKR_NOEXCEPT
    {
        _canvas->path_end();
    }

private:
    ICanvas* _canvas;
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

} // namespace skr::gui
