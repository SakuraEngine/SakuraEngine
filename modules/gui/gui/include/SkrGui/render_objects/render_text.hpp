#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/color.hpp"
// Avoid including type/type.hpp by including "containers/detail/sptr.hpp" instead.
// #include "containers/sptr.hpp"
#include "containers/detail/sptr.hpp"
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
    float        font_size;
    skr_float4_t color;
};

struct SKR_GUI_API BindText {
    String text;
};
struct InlineType : public std::variant<skr::string, RenderObject*, RenderText*, skr::SPtr<BindText>> {
};

struct SKR_GUI_API RenderText : public RenderBox {
public:
    SKR_GUI_TYPE(RenderText, "72e8d4de-c288-4675-a22f-4c7a6487cabd", RenderBox);
    using Super = RenderBox;

    RenderText();
    virtual ~RenderText();

    void perform_layout() SKR_NOEXCEPT override;
    void paint(NotNull<PaintingContext*> context, Offset offset) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}

    void add_text(const char8_t* u8_text);

protected:
    void BuildParagraph();
    void DrawParagraph();

private:
    void buildParagraphRec(Paragraph* p, const StyleText& txt);

    Array<InlineType>   inlines_ = {};
    Paragraph*          paragraph_ = nullptr;
    skr::SPtr<FontFile> font_ = nullptr;

    bool  paragraph_dirty_ = true;
    Color font_color = { 1.f, 0.f, 1.f, 1.f };
};

} // namespace skr::gui
