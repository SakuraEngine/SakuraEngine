#include "SkrGui/render_elements/render_text.hpp"

namespace skr {
namespace gui {

RenderText::RenderText()
{

}

RenderText::~RenderText()
{

}

void RenderText::layout(Constraints* constraints, bool needSize)
{

}

void RenderText::draw(skr_gdi_viewport_id viewport, skr_gdi_canvas_id canvas)
{
    RenderElement::draw(viewport, canvas);
    
}

skr_float2_t RenderText::get_size() const
{
    return skr_float2_t();
}

void RenderText::set_size(const skr_float2_t& size)
{

}


} }