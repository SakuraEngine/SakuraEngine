#include "SkrGui/backend/canvas.hpp"

// TODO. use math
#define SKR_PI 3.14159265358979323846264338327950288f

namespace skr::gui
{
ICanvas::ICanvas(GDIDevice* device) SKR_NOEXCEPT
    : _gdi_device(device),
      _gdi_canvas(nullptr),
      _current_gdi_element(nullptr),
      _gdi_elements(),
      _state_stack()
{
    SKR_GUI_ASSERT(_gdi_device != nullptr);

    _gdi_canvas = _gdi_device->create_canvas();
    _state_stack.push_back({}); // default stack layer
}

ICanvas::~ICanvas() SKR_NOEXCEPT
{
    SKR_GUI_ASSERT(_gdi_device != nullptr);

    if (_gdi_canvas)
    {
        _gdi_device->free_canvas(_gdi_canvas);
        _gdi_canvas = nullptr;
    }

    for (auto& element : _gdi_elements)
    {
        _gdi_device->free_element(element);
    }
    _gdi_elements.clear();
}

//==> paint scope
void ICanvas::paint_begin(float pixel_ratio) SKR_NOEXCEPT
{
    if (_current_gdi_element)
    {
        SKR_GUI_LOG_ERROR("ICanvas::paint_begin() called without a matching ICanvas::paint_end() call");
        _gdi_canvas->add_element(_current_gdi_element);
        _current_gdi_element = nullptr;
    }

    _current_gdi_element = _gdi_device->create_element();
    _gdi_elements.push_back(_current_gdi_element);
    _current_gdi_element->begin_frame(pixel_ratio);
    _is_in_paint_scope = true;
}
void ICanvas::paint_end() SKR_NOEXCEPT
{
    // add to canvas
    if (!_current_gdi_element)
    {
        SKR_GUI_LOG_ERROR("ICanvas::paint_end() called without a matching ICanvas::paint_begin() call");
    }
    else
    {
        _gdi_canvas->add_element(_current_gdi_element);
        _current_gdi_element = nullptr;
    }

    // reset state stack
    if (_state_stack.size() > 1)
    {
        while (_state_stack.size() > 1)
        {
            state_restore();
        }
        SKR_GUI_LOG_ERROR("state stack should be empty before paint_end() call, please check ICanvas::state_save() and ICanvas::state_restore() calls");
    }

    __internal_repair_state_stack_if_need();

    // reset stack
    _state_stack.back() = {};
    _is_in_paint_scope = false;
}
CanvasPaintScope ICanvas::paint_scope(float pixel_ratio) SKR_NOEXCEPT
{
    if (!_is_in_paint_scope)
    {
        SKR_GUI_LOG_ERROR("ICanvas::paint_scope() called without a matching ICanvas::paint_end() call");
    }
    return { this, pixel_ratio };
}

//==> states stack
void ICanvas::state_save() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        _current_gdi_element->save();
        _state_stack.push_back({});
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
        _current_gdi_element->restore();
        if (_state_stack.size() >= 1)
        {
            _state_stack.pop_back();
        }
        else
        {
            __internal_repair_state_stack_if_need();
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_restore() called without a matching ICanvas::state_save() call");
    }
}
CanvasStateScope ICanvas::state_scope() SKR_NOEXCEPT
{
    if (!_is_in_paint_scope)
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_scope() called without a matching ICanvas::state_save() call");
    }
    return (this);
}

//==> vg states
void ICanvas::state_reset() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        // TODO. impl state_reset
        SKR_ASSERT(false && "ICanvas::state_reset() not implemented yet");
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_reset() called outside of a paint scope");
    }
}
void ICanvas::state_paint_style(EPaintStyle style) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        _state_stack.back().paint_style = style;
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_paint_style() called outside of a paint scope");
    }
}
void ICanvas::state_stroke_cap(EStrokeCap cap) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        // TODO. impl state_stroke_cap
        SKR_ASSERT(false && "ICanvas::state_stroke_cap() not implemented yet");
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_stroke_cap() called outside of a paint scope");
    }
}
void ICanvas::state_stroke_join(EStrokeJoin join) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        // TODO. impl state_stroke_join
        SKR_ASSERT(false && "ICanvas::state_stroke_join() not implemented yet");
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_stroke_join() called outside of a paint scope");
    }
}
void ICanvas::state_stroke_width(float width) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        _current_gdi_element->stroke_width(width);
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_stroke_width() called outside of a paint scope");
    }
}
void ICanvas::state_stroke_miter_limit(float limit) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        // TODO. impl state_stroke_miter_limit
        SKR_ASSERT(false && "ICanvas::state_stroke_miter_limit() not implemented yet");
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_stroke_miter_limit() called outside of a paint scope");
    }
}
void ICanvas::state_anti_alias(bool enable) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        // TODO. impl in GDI
        __internal_repair_state_stack_if_need();
        _state_stack.back().anti_alias = enable;
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_anti_alias() called outside of a paint scope");
    }
}

//==> transform
void ICanvas::state_transform_reset() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        // TODO. impl state_transform_reset
        SKR_ASSERT(false && "ICanvas::state_transform_reset() not implemented yet");
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_transform_reset() called outside of a paint scope");
    }
}
void ICanvas::state_translate(Offset offset) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        _current_gdi_element->translate(offset.x, offset.y);
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
        _current_gdi_element->rotate(degree * SKR_PI / 180.0f);
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
        // TODO. impl state_scale
        SKR_ASSERT(false && "ICanvas::state_scale() not implemented yet");
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_scale() called outside of a paint scope");
    }
}
void ICanvas::state_skew_x(float skew) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        // TODO. impl state_skew_x
        SKR_ASSERT(false && "ICanvas::state_skew_x() not implemented yet");
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_skew_x() called outside of a paint scope");
    }
}
void ICanvas::state_skew_y(float skew) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        // TODO. impl state_skew_y
        SKR_ASSERT(false && "ICanvas::state_skew_y() not implemented yet");
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_skew_y() called outside of a paint scope");
    }
}

//==> paint states
void ICanvas::state_paint_reset() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        __internal_repair_state_stack_if_need();
        _state_stack.back() = {};
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_paint_reset() called outside of a paint scope");
    }
}
ColorPaintBuilder ICanvas::state_paint_color(Color color) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        __internal_repair_state_stack_if_need();
        _state_stack.back().paint_type = EPaintType::Color;
        _state_stack.back().color = color;
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_paint_color() called outside of a paint scope");
    }
    return { this };
}
TexturePaintBuilder ICanvas::state_paint_texture(ITexture* texture) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        __internal_repair_state_stack_if_need();
        _state_stack.back().paint_type = EPaintType::Texture;
        _state_stack.back().texture = texture;
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_paint_texture() called outside of a paint scope");
    }
    return { this };
}
MaterialPaintBuilder ICanvas::state_paint_material(IMaterial* material) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        __internal_repair_state_stack_if_need();
        _state_stack.back().paint_type = EPaintType::Material;
        _state_stack.back().material = material;
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::state_paint_material() called outside of a paint scope");
    }
    return { this };
}

//==> path
void ICanvas::path_begin() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        _current_gdi_element->begin_path();
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_begin() called outside of a paint scope");
    }

    _is_in_path_scope = true;
}
void ICanvas::path_end() SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        __internal_repair_state_stack_if_need();
        auto& paint_state = _state_stack.back();
        // TODO. call fill()/stroke() api
    }
    else
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_end() called outside of a paint scope");
    }

    _is_in_path_scope = false;
}
CanvasPathScope ICanvas::path_scope() SKR_NOEXCEPT
{
    if (!_is_in_paint_scope)
    {
        SKR_GUI_LOG_ERROR("ICanvas::path_scope() called outside of a paint scope");
    }
    return { this };
}

//==> custom path
void ICanvas::path_move_to(Offset to) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            _current_gdi_element->move_to(to.x, to.y);
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
void ICanvas::path_line_to(Offset to) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            _current_gdi_element->line_to(to.x, to.y);
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
void ICanvas::path_quad_to(Offset to, Offset control_point) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            // TODO. impl path_quad_to
            SKR_ASSERT(false && "ICanvas::path_quad_to() not implemented yet");
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
void ICanvas::path_cubic_to(Offset to, Offset control_point1, Offset control_point2) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            // TODO. impl path_cubic_to
            SKR_ASSERT(false && "ICanvas::path_cubic_to() not implemented yet");
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
void ICanvas::path_arc_to(Offset to, Offset control_point, float radius) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            // TODO. impl path_arc_to
            SKR_ASSERT(false && "ICanvas::path_arc_to() not implemented yet");
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
            _current_gdi_element->close_path();
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

//==> simple shape path
void ICanvas::path_arc(Offset center, float radius, float start_degree, float end_degree) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            _current_gdi_element->arc(center.x, center.y, radius, start_degree, end_degree, ::skr::gdi::EGDIWinding::CW);
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
void ICanvas::path_rect(Rect rect) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            _current_gdi_element->rect(rect.left, rect.top, rect.width(), rect.height());
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
void ICanvas::path_circle(Offset center, float radius) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            _current_gdi_element->circle(center.x, center.y, radius);
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
void ICanvas::path_ellipse(Offset center, float radius_x, float radius_y) SKR_NOEXCEPT
{
    if (_is_in_paint_scope)
    {
        if (_is_in_path_scope)
        {
            // TODO. impl path_ellipse
            SKR_ASSERT(false && "ICanvas::path_ellipse() not implemented yet");
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

//==> helper draw, warper for path
void ICanvas::draw_arc(Offset center, float radius, float start_degree, float end_degree) SKR_NOEXCEPT
{
    auto _ = this->path_scope();
    this->path_arc(center, radius, start_degree, end_degree);
}
void ICanvas::draw_rect(Rect rect) SKR_NOEXCEPT
{
    auto _ = this->path_scope();
    this->path_rect(rect);
}
void ICanvas::draw_circle(Offset center, float radius) SKR_NOEXCEPT
{
    auto _ = this->path_scope();
    this->path_circle(center, radius);
}
void ICanvas::draw_ellipse(Offset center, float radius_x, float radius_y) SKR_NOEXCEPT
{
    auto _ = this->path_scope();
    this->path_ellipse(center, radius_x, radius_y);
}

// help
bool ICanvas::__internal_repair_state_stack_if_need() SKR_NOEXCEPT
{
    if (_state_stack.size() < 1)
    {
        SKR_GUI_LOG_ERROR("state stack not balanced, restoring to default state");
        _state_stack.push_back({}); // default stack layer
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace skr::gui