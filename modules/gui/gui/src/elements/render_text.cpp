#include "SkrGui/render_elements/render_text.hpp"

namespace skr {
namespace gui {

RenderText::RenderText()
{
    diagnostic_builder.add_properties(
        SkrNew<TextDiagnosticProperty>("type", "text", "draws text paragraph")
    );
}

RenderText::~RenderText()
{

}

void RenderText::layout(Constraints* constraints, bool needSize)
{

}

void RenderText::draw(const DrawParams* params)
{
    RenderElement::draw(params);
    
}

skr_float2_t RenderText::get_size() const
{
    return skr_float2_t();
}

void RenderText::set_size(const skr_float2_t& size)
{

}


} }