#pragma once
#include "SkrGui/backend/text/paragraph.hpp"
#include "backend/text_server/text_paragraph.h"

namespace godot
{
class TextParagraph;
}

namespace skr::gui
{
struct _EmbeddedParagraph : public godot::TextParagraph, public IParagraph {
    SKR_GUI_OBJECT(_EmbeddedParagraph, "1d611491-1e27-42cf-9604-4135b6617e21", IParagraph)

    _EmbeddedParagraph();

    void  clear() override;
    void  build() override;
    void  add_text(const String& text, const TextStyle& style) override;
    Sizef layout(BoxConstraints constraints) override;
    void  paint(NotNull<PaintingContext*> context, Offsetf offset) override;

private:
    void _draw(godot::TextServer::TextDrawProxy* proxy, const skr_float2_t& p_pos, const godot::Color& p_color, const godot::Color& p_dc_color);

private:
    Array<String> _texts = {}; // TODO. inline
    bool          _dirty = false;
};
} // namespace skr::gui