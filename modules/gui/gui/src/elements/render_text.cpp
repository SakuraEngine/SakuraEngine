#include "SkrGui/render_elements/render_text.hpp"
#include "text_server/text_paragraph.h"
#include "text_server/font.h"
#include <containers/sptr.hpp>
#include <variant>
#include <fstream>

namespace skr {
namespace gui {

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct InlineType : public std::variant<skr::text::text, RenderElement*, RenderText*, skr::SPtr<BindText>> {};

struct Paragraph : public godot::TextParagraph
{
    void draw(const skr_float2_t& p_pos, const godot::Color &p_color, const godot::Color& p_dc_color)
    {
        const uint32_t max_width = UINT32_MAX;
        const float line_height_scale = 1.f;
	    const int spacing_top = 0;
	    const int spacing_bottom = 0;
        godot::RID p_canvas = {};

        _shape_lines();
        godot::Vector2 ofs = { p_pos.x, p_pos.y };
        float h_offset = 0.f;
        if (godot::TS->shaped_text_get_orientation(dropcap_rid) == godot::TextServer::ORIENTATION_HORIZONTAL) {
            h_offset = godot::TS->shaped_text_get_size(dropcap_rid).x + dropcap_margins.size.x + dropcap_margins.position.x;
        } else {
            h_offset = godot::TS->shaped_text_get_size(dropcap_rid).y + dropcap_margins.size.y + dropcap_margins.position.y;
        }

        if (h_offset > 0) {
            // Draw dropcap.
            godot::Vector2 dc_off = { ofs.x, ofs.y };
            if (godot::TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_RTL) {
                if (godot::TS->shaped_text_get_orientation(dropcap_rid) == godot::TextServer::ORIENTATION_HORIZONTAL) {
                    dc_off.x += (float)max_width - h_offset;
                } else {
                    dc_off.y += (float)max_width - h_offset;
                }
            }
            godot::Vector2 p_pos = { dc_off.x, dc_off.y };
            p_pos.y += godot::TS->shaped_text_get_ascent(dropcap_rid) + dropcap_margins.size.y + dropcap_margins.position.y / 2;
            godot::TS->shaped_text_draw(dropcap_rid, p_canvas, p_pos, -1, -1, p_dc_color);
        }

        int lines_visible = (max_lines_visible >= 0) ? MIN(max_lines_visible, lines_rid.size()) : lines_rid.size();

        for (int i = 0; i < lines_visible; i++) {
            float l_width = (float)max_width;
            if (godot::TS->shaped_text_get_orientation(lines_rid[i]) == godot::TextServer::ORIENTATION_HORIZONTAL) {
                ofs.x = p_pos.x;
                ofs.y += godot::TS->shaped_text_get_ascent(lines_rid[i]) * line_height_scale + spacing_top;
                if (i < dropcap_lines) {
                    if (godot::TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_LTR ||
                        godot::TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_AUTO) {
                        ofs.x += h_offset;
                    }
                    l_width -= h_offset;
                }
            } else {
                ofs.y = p_pos.y;
                ofs.x += godot::TS->shaped_text_get_ascent(lines_rid[i]) * line_height_scale + spacing_top;
                if (i < dropcap_lines) {
                    if (godot::TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_LTR || 
                        godot::TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_AUTO) 
                    {
                        ofs.x += h_offset;
                    }
                    l_width -= h_offset;
                }
            }
            float line_width = godot::TS->shaped_text_get_width(lines_rid[i]);
            if (max_width > 0) {
                float offset = 0.f;
                switch (alignment) {
                    case godot::HORIZONTAL_ALIGNMENT_FILL:
                    case godot::HORIZONTAL_ALIGNMENT_LEFT:
                        break;
                    case godot::HORIZONTAL_ALIGNMENT_CENTER: 
                        offset = std::floor((l_width - line_width) / 2.0);
                        break;
                    case godot::HORIZONTAL_ALIGNMENT_RIGHT: 
                        offset = l_width - line_width;
                        break;
                }
                
                if (godot::TS->shaped_text_get_orientation(lines_rid[i]) == godot::TextServer::ORIENTATION_HORIZONTAL) {
                    ofs.x += offset;
                } else {
                    ofs.y += offset;
                }
            }
            float clip_l;
            if (godot::TS->shaped_text_get_orientation(lines_rid[i]) == godot::TextServer::ORIENTATION_HORIZONTAL) {
                clip_l = MAX(0, p_pos.x - ofs.x);
            } else {
                clip_l = MAX(0, p_pos.y - ofs.y);
            }
            godot::TS->shaped_text_draw(lines_rid[i], p_canvas, ofs, clip_l, clip_l + l_width, p_color);
            if (godot::TS->shaped_text_get_orientation(lines_rid[i]) == godot::TextServer::ORIENTATION_HORIZONTAL) {
                ofs.x = p_pos.x;
                ofs.y += godot::TS->shaped_text_get_descent(lines_rid[i]) * line_height_scale + spacing_bottom;
            } else {
                ofs.y = p_pos.y;
                ofs.x += godot::TS->shaped_text_get_descent(lines_rid[i]) * line_height_scale + spacing_bottom;
            }
        }
    }
};

struct FontFile : public godot::FontFile
{
    
};

godot::InlineAlignment GetInlineAlignment(EInlineAlignment o)
{
    switch(o)
    {
        case EInlineAlignment::Baseline:
            return godot::INLINE_ALIGNMENT_BOTTOM;
        case EInlineAlignment::Top:
            return godot::INLINE_ALIGNMENT_TOP;
        case EInlineAlignment::Middle:
            return godot::INLINE_ALIGNMENT_CENTER;
    }
    return godot::INLINE_ALIGNMENT_BOTTOM;
}

StyleText TODO_StyleText = {
    12.0f,
    {1.0f, 1.0f, 1.0f, 1.0f}
};


RenderText::RenderText(skr_gdi_device_id gdi_device)
    : RenderBox(gdi_device)
{
    diagnostic_builder.add_properties(
        SkrNew<TextDiagnosticProperty>("type", "text", "draws text paragraph")
    );

    paragraph_ = SkrNew<Paragraph>();
    font_ = SPtr<FontFile>::Create();
    godot::PackedByteArray data = {};

    std::fstream file("./../resources/font/SourceSansPro-Regular.ttf", std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        data.resize(size);
        file.read((char*)data.ptrw(), size);
        file.close();
    }
    font_->set_data(data);
}

RenderText::~RenderText()
{
    SkrDelete(paragraph_);
}

void RenderText::layout(BoxConstraint constraints, bool needSize)
{
    size.x = eastl::clamp(size.x, constraints.min_size.x, constraints.max_size.x);
    size.y = eastl::clamp(size.y, constraints.min_size.y, constraints.max_size.y);
}

void RenderText::draw(const DrawParams* params)
{
    RenderBox::draw(params);

    BuildParagraph();
    DrawParagraph();
}

void RenderText::add_text(const char* u8_text)
{
    inlines_.get().emplace_back(InlineType{ skr::text::text(u8_text) });
    paragraph_dirty_ = true;
}

void RenderText::DrawParagraph()
{
    godot::Color p_color = { 1.f, 1.f, 1.f} ;
    godot::Color p_dc_color = { 1.f, 1.f, 1.f} ;
    paragraph_->draw(pos, p_color, p_dc_color);
}

void RenderText::BuildParagraph()
{
    if(paragraph_dirty_)
    {
        paragraph_->clear();
        
        // auto& txt = StyleText::Get(_style);
        auto& txt = TODO_StyleText;
        buildParagraphRec(paragraph_, txt);
        MarkLayoutDirty(false);
        paragraph_dirty_ = false;
    }
}

void RenderText::buildParagraphRec(Paragraph* p, const StyleText& txt)
{
    auto& inlines = inlines_.get();
    for(auto& inl : inlines)
    {
        std::visit(overloaded
        {
            [&](skr::text::text& text) 
            { 
                godot::Color color(txt.color.x, txt.color.y, txt.color.z, txt.color.w);
                /*
                godot::Color decorationColor(txt.textDecorationColor.X, txt.textDecorationColor.Y, txt.textDecorationColor.Z, txt.textDecorationColor.W);
                godot::TextDecorationData decoration;
                decoration.decorationColor = decorationColor;
                decoration.decorationTexture = nullptr;
                decoration.decorationLineFlag = (int64_t)txt.textDecorationLine;
                decoration.decorationThickness = txt.textDecorationThickness;
                int64_t flags = 0;
                */
                auto font = static_pointer_cast<godot::Font>(font_);
                auto ft = godot::Ref<godot::Font>(font);
                p->add_string(text.c_str(), ft, txt.font_size, "", {}); 
            },
            [&](RenderElement*& child) 
            { 
                // if(!child->Visible()) return;
                // auto& pos = StylePosition::Get(child->_style);
                const auto TODO = EInlineAlignment::Middle;
                p->add_object(child, {0, 0}, GetInlineAlignment(TODO)); 
            },
            [&](RenderText*& child) 
            { 
                // if(!child->Visible()) return;
                // StyleText& ctxt = StyleText::Get(child->_style);
                StyleText& ctxt = TODO_StyleText;
                child->buildParagraphRec(p, ctxt);
            },
            [&](skr::SPtr<BindText>& Bind) 
            { 
                godot::Color color(txt.color.x, txt.color.y, txt.color.z, txt.color.w);
                /*
                godot::Color decorationColor(txt.textDecorationColor.X, txt.textDecorationColor.Y, txt.textDecorationColor.Z, txt.textDecorationColor.W);
                godot::TextDecorationData decoration;
                decoration.decorationColor = decorationColor;
                decoration.decorationTexture = nullptr;
                decoration.decorationLineFlag = (int64_t)txt.textDecorationLine;
                decoration.decorationThickness = txt.textDecorationThickness;
                */
                auto font = static_pointer_cast<godot::Font>(font_);
                auto ft = godot::Ref<godot::Font>(font);
                paragraph_->add_string((wchar_t*)Bind->text.get().c_str(), ft, txt.font_size, "", draw_policy_);
            }
        }, inl);
    }
}

} }