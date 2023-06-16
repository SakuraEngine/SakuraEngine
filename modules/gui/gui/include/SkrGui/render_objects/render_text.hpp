#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
// Avoid including type/type.hpp by including "containers/detail/sptr.hpp" instead.
// #include "containers/sptr.hpp"
#include "containers/detail/sptr.hpp"

namespace skr::gdi
{
struct IGDIDevice;
struct IGDIElement;
struct IGDIPaint;
} // namespace skr::gdi
namespace skr::gui
{

struct Paragraph;
struct FontFile;
struct InlineType;

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

struct SKR_GUI_API RenderText : public RenderBox {
public:
    SKR_GUI_TYPE(RenderText, "72e8d4de-c288-4675-a22f-4c7a6487cabd", RenderBox);
    RenderText(gdi::IGDIDevice* gdi_device);
    virtual ~RenderText();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    void add_text(const char8_t* u8_text);

protected:
    void BuildParagraph();
    void DrawParagraph();
    void MarkLayoutDirty(bool visibility){};

private:
    void buildParagraphRec(Paragraph* p, const StyleText& txt);

    Array<struct InlineType> inlines_;
    Paragraph*               paragraph_ = nullptr;
    skr::SPtr<FontFile>      font_ = nullptr;

    bool              paragraph_dirty_ = true;
    gdi::IGDIDevice*  gdi_device = nullptr;
    gdi::IGDIElement* gdi_element = nullptr;
    gdi::IGDIPaint*   gdi_paint = nullptr;
    skr_float4_t      font_color = { 1.f, 0.f, 1.f, 1.f };
};

} // namespace skr::gui
