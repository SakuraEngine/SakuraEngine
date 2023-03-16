#include "SkrGui/render_elements/render_text.hpp"

namespace skr {
namespace gui {

RenderText::RenderText(skr_gdi_device_id gdi_device)
    : RenderBox(gdi_device)
{
    diagnostic_builder.add_properties(
        SkrNew<TextDiagnosticProperty>("type", "text", "draws text paragraph")
    );
}

RenderText::~RenderText()
{

}

void RenderText::layout(BoxConstraint constraints, bool needSize)
{

}

void RenderText::draw(const DrawParams* params)
{
    RenderBox::draw(params);
    
}

} }