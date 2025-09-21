#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/color.hpp"
#include "SkrCore/memory/sp.hpp"
#include <variant> // TODO. use skr::variant, here for shit msvc

#include "SkrGui/render_objects/render_text.generated.h"

namespace skr::gui
{
struct RenderText;
struct IParagraph;
struct FontFile;

enum class EInlineAlignment : uint32_t
{
    Baseline,
    Top,
    Middle
};

struct StyleText
{
    float font_size = 14.0f;
    float4 color = {};
};

struct SKR_GUI_API BindText
{
    String text = {};
};
struct InlineType : public std::variant<skr::String, RenderObject*, RenderText*, skr::SP<BindText>>
{
};

struct [[sattr(guid = "5179c185-bc7f-4f12-9c11-d979fc14e515"
)]] SKR_GUI_API RenderText : public RenderBox
{
public:
    SKR_GENERATE_BODY(RenderText)
    using Super = RenderBox;

    RenderText();
    ~RenderText();

    void perform_layout() SKR_NOEXCEPT override;
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}

    void set_text(const String& text);
    const String& text() const { return _text; }

private:
    IParagraph* _paragraph = nullptr;
    String _text = {};
};
} // namespace skr::gui
