#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/color.hpp"
// Avoid including type/type.hpp by including "containers/detail/sptr.hpp" instead.
// #include "SkrRT/containers/sptr.hpp"
#include "SkrRT/containers/detail/sptr.hpp"
#include <variant> // TODO. use skr::variant, here for shit msvc

#ifndef __meta__
    #include "SkrGui/render_objects/render_text.hpp"
#endif

namespace skr sreflect
{
namespace gui sreflect
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

sreflect_struct(
    "guid": "5179c185-bc7f-4f12-9c11-d979fc14e515"
)
SKR_GUI_API RenderText : public RenderBox {
public:
    SKR_RTTR_GENERATE_BODY()
    using Super = RenderBox;

    RenderText() {}
    ~RenderText() {}

    void perform_layout() SKR_NOEXCEPT override {}
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override {}
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}
};
} // namespace gui sreflect
} // namespace skr sreflect
