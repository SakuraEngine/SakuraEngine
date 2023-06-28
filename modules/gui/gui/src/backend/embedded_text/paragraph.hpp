#include "SkrGui/backend/text/paragraph.hpp"

namespace godot
{
class TextParagraph;
}

namespace skr::gui
{
struct _EmbeddedTextService;
struct _SkrGodotParagraph;

struct _EmbeddedParagraph : public IParagraph {
    SKR_GUI_OBJECT(_EmbeddedParagraph, "1d611491-1e27-42cf-9604-4135b6617e21", IParagraph)

    _EmbeddedParagraph(_EmbeddedTextService* service);

    void  clear() override;
    void  build() override;
    void  add_text(const String& text, const TextStyle& style) override;
    Sizef layout(BoxConstraints constraints) override;
    void  paint(NotNull<PaintingContext*> context, Offsetf offset) override;

private:
    _EmbeddedTextService* _service = nullptr;
    _SkrGodotParagraph*   _paragraph = nullptr;
    Array<String>         _texts = {}; // TODO. inline
    bool                  _dirty = false;
};
} // namespace skr::gui