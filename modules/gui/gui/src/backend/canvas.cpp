#include "SkrGui/backend/canvas.hpp"

namespace skr::gui
{
ICanvas::ICanvas(GDIDevice* device) SKR_NOEXCEPT
    : _gdi_device(device),
      _gdi_canvas(nullptr),
      _current_gdi_element(nullptr),
      _gdi_elements(),
      _gdi_paint_stack(),
      _paint_style(EPaintStyle::Fill)
{
    SKR_GUI_ASSERT(_gdi_device != nullptr);

    _gdi_canvas = _gdi_device->create_canvas();
}

ICanvas::~ICanvas() SKR_NOEXCEPT
{
    SKR_GUI_ASSERT(_gdi_device != nullptr);

    if (_gdi_canvas)
    {
        _gdi_device->free_canvas(_gdi_canvas);
        _gdi_canvas = nullptr;
    }

    if (_gdi_paint)
    {
        _gdi_device->free_paint(_gdi_paint);
        _gdi_paint = nullptr;
    }

    for (auto& element : _gdi_elements)
    {
        _gdi_device->free_element(element);
    }
    _gdi_elements.clear();
}

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
}
void ICanvas::paint_end() SKR_NOEXCEPT
{
    if (!_current_gdi_element)
    {
        SKR_GUI_LOG_ERROR("ICanvas::paint_end() called without a matching ICanvas::paint_begin() call");
    }
    else
    {
        _gdi_canvas->add_element(_current_gdi_element);
        _current_gdi_element = nullptr;
    }
}
CanvasPaintScope ICanvas::paint_scope(float pixel_ratio) SKR_NOEXCEPT { return CanvasPaintScope(this, pixel_ratio); }

} // namespace skr::gui