#pragma once
#include "SkrGui/backend/text/paragraph.hpp"
#include "backend/text_server/text_paragraph.h"
#include "SkrGui/_private/paragraph.generated.h"

namespace godot
{
class TextParagraph;
}

// 为了过编
SKR_TYPE_INFO(godot::TextParagraph, "c15eb17d-0444-42c3-a753-240de4a1443c")

namespace skr::gui
{
struct [[sattr(guid = "4863f5b6-c952-468d-9460-1a5841f2d8f5")]]
_EmbeddedParagraph : public godot::TextParagraph,
                     public IParagraph {
    SKR_GENERATE_BODY(_EmbeddedParagraph)

    _EmbeddedParagraph();

    void  clear() override;
    void  build() override;
    void  add_text(const String& text, const TextStyle& style) override;
    Sizef layout(BoxConstraints constraints) override;
    void  paint(NotNull<PaintingContext*> context, Offsetf offset) override;

private:
    void _draw(godot::TextServer::TextDrawProxy* proxy, const float2& p_pos, const godot::Color& p_color, const godot::Color& p_dc_color);

private:
    Array<String> _texts = {}; // TODO. inline
    bool          _dirty = false;
};
} // namespace skr::gui