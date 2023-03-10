#include "SkrGui/render_elements/render_box.hpp"

namespace skr {
namespace gui {

RenderBox::RenderBox()
{

}

RenderBox::~RenderBox()
{

}

void RenderBox::layout(Constraints* constraints, bool needSize)
{

}

void RenderBox::draw(const DrawParams* params)
{
    RenderElement::draw(params);
}

skr_float2_t RenderBox::get_size() const
{
    return skr_float2_t();
}

void RenderBox::set_size(const skr_float2_t& size)
{
    
}


} }