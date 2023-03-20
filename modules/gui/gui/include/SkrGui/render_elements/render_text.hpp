#pragma once
#include "SkrGui/framework/render_box.hpp"
// Avoid including type/type.hpp by including "containers/detail/sptr.hpp" instead.
// #include "containers/sptr.hpp"
#include "containers/detail/sptr.hpp"

namespace skr {
namespace gui {

struct Paragraph;
struct Font;
struct InlineType;

enum class EInlineAlignment : uint32_t
{
    Baseline,
    Top,
    Middle
};

struct StyleText
{
    float font_size;
    skr_float4_t color;
};

struct SKR_GUI_API BindText
{
    TextStorage text;
};

struct SKR_GUI_API RenderText : public RenderBox
{
public:
    RenderText(skr_gdi_device_id gdi_device);
    virtual ~RenderText();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    void add_text(const char* u8_text);

protected:
    void BuildParagraph();
    void DrawParagraph();
    void MarkLayoutDirty(bool visibility) {};

private:
    void buildParagraphRec(Paragraph* p, const StyleText& txt);

    VectorStorage<struct InlineType> inlines_;
    Paragraph* paragraph_ = nullptr;
    skr::SPtr<Font> font_ = nullptr;

    bool paragraph_dirty_ = true;
    void* draw_policy_ = nullptr;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderText, skr_gui_render_text);