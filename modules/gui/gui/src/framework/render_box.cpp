#include "SkrGui/framework/render_box.hpp"
#include "SkrGui/gdi/gdi.hpp"
#include <algorithm>

namespace skr {
namespace gui {

    
RenderBoxSizeType BoxConstraint::apply(const RenderBoxSizeType& size) const
{
    RenderBoxSizeType result = size;
    result.x = std::max(min_size.x, std::min(max_size.x, result.x));
    result.y = std::max(min_size.y, std::min(max_size.y, result.y));
    return result;
}

RenderBox::RenderBox(skr_gdi_device_id gdi_device)
    : gdi_device(gdi_device), debug_element(nullptr)
{
    diagnostic_builder.add_properties(
        SkrNew<BoolDiagnosticProperty>("render_box", true, "")
    );
    debug_element = gdi_device->create_element();
}

RenderBox::~RenderBox()
{
    gdi_device->free_element(debug_element);
}

void RenderBox::before_draw(const DrawParams* params)
{
    RenderElement::before_draw(params);

    if (auto canvas = draw_debug_rect ? params->canvas : nullptr)
    {
        debug_element->begin_frame(1.f);
        {            
            debug_element->begin_path();
            // actual region
            debug_element->fill_color(128u, 0u, 0u, 128u);
            debug_element->rect(pos.x, pos.y, size.x, size.y);
            debug_element->fill();
            
            // padding region
            debug_element->begin_path();
            debug_element->fill_color(128u, 0u, 0u, 64u);
            const float Rate = 0.01f;
            float CanvasW, CanvasH; canvas->get_size(&CanvasW, &CanvasH);
            const auto DistX = Rate * std::min(CanvasW, CanvasH);
            const auto DistY = Rate * std::min(CanvasW, CanvasH);
            auto X = pos.x - DistX; const auto W = size.x + 2 * DistX;
            auto Y = pos.y - DistY; const auto H = size.y + 2 * DistY;
            debug_element->rect(X, Y, W, H);
            debug_element->fill();
        }
        addElementToCanvas(params, debug_element);
    }
}

void RenderBox::draw(const DrawParams* params)
{
    RenderElement::draw(params);
}

RenderBoxSizeType RenderBox::get_size() const
{
    return size;
}

void RenderBox::set_size(const RenderBoxSizeType& in_size)
{
    size = in_size;
}

void RenderBox::set_position(const RenderBoxSizeType& in_pos)
{
    pos = in_pos;
}

void RenderBox::enable_debug_draw(bool enable)
{
    draw_debug_rect = enable;
}

} }