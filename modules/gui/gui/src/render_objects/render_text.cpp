#include "SkrGui/render_objects/render_text.hpp" // IWYU pragma: keep
#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/backend/text/paragraph.hpp"
#include "SkrGui/backend/text/text_style.hpp"

namespace skr::gui
{
RenderText::RenderText()
{
    // TODO. use window's font service
    _paragraph = embedded_create_paragraph();
}
RenderText::~RenderText()
{
    // TODO. use window's font service
    embedded_destroy_paragraph(_paragraph);
}

void RenderText::perform_layout() SKR_NOEXCEPT
{
    _paragraph->clear();
    _paragraph->add_text(_text, {});
    _paragraph->build();
    set_size(_paragraph->layout(constraints()));
}
void RenderText::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    _paragraph->paint(context, offset);
}

void RenderText::set_text(const skr::String& text)
{
    if (text != _text)
    {
        _text = text;
        mark_needs_layout();
        mark_needs_paint();
    }
}
} // namespace skr::gui