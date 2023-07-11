#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/color.hpp"
// Avoid including type/type.hpp by including "containers/detail/sptr.hpp" instead.
// #include "SkrRT/containers/sptr.hpp"
#include "SkrRT/containers/detail/sptr.hpp"
#include <variant> // TODO. use skr::variant, here for shit msvc

namespace skr::gui
{

struct RenderText;
struct Paragraph;
struct FontFile;

enum class EInlineAlignment : uint32_t
{
    Baseline,
    Top,
    Middle
};

struct StyleText {
    float        font_size = 14.0f;
    skr_float4_t color     = {};
};

struct SKR_GUI_API BindText {
    String text = {};
};
struct InlineType : public std::variant<skr::string, RenderObject*, RenderText*, skr::SPtr<BindText>> {
};

struct SKR_GUI_API RenderText : public RenderBox {
public:
    SKR_GUI_OBJECT(RenderText, "72e8d4de-c288-4675-a22f-4c7a6487cabd", RenderBox);
    using Super = RenderBox;

    RenderText() {}
    ~RenderText() {}

    void perform_layout() SKR_NOEXCEPT override {}
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override {}
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}
};

} // namespace skr::gui
