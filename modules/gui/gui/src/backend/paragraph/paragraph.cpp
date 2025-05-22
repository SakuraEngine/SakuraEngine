#include "SkrCore/memory/sp.hpp"
#include "SkrGui/backend/canvas/canvas.hpp"
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/_private/paragraph.hpp"
#include "backend/text_server/text_paragraph.h"
#include "backend/text_server/font.h"

// TODO. font service
#include <fstream>
namespace skr::gui
{
static SP<godot::FontFile> _font = nullptr;
void                       _init_font()
{
    if (!_font)
    {
        std::fstream file("./../resources/font/SourceSansPro-Regular.ttf", std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            godot::PackedByteArray data = {};

            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);
            data.resize(size);
            file.read((char*)data.ptrw(), size);
            file.close();

            _font = SP<godot::FontFile>::New();
            _font->set_data(data);
        }
    }
}
} // namespace skr::gui

namespace skr::gui
{
struct _SkrGodotParagraph : public godot::TextParagraph {
};
} // namespace skr::gui

namespace skr::gui
{

_EmbeddedParagraph::_EmbeddedParagraph()
{
    // TODO. font service
    _init_font();
}

void _EmbeddedParagraph::clear()
{
    _texts.clear();
}
void _EmbeddedParagraph::build()
{
    if (_dirty)
    {
        godot::TextParagraph::clear();

        for (const auto& text : _texts)
        {
            auto font = _font.cast_static<godot::Font>();
            auto ft   = godot::Ref<godot::Font>(font);
            this->add_string(godot::String::utf8(text.c_str_raw()), ft, 42, "", {});
        }

        _shape_lines();
        _dirty = false;
    }
}
void _EmbeddedParagraph::add_text(const String& text, const TextStyle& style)
{
    _texts.add(text);
    _dirty = true;
}
Sizef _EmbeddedParagraph::layout(BoxConstraints constraints)
{
    build();
    this->set_width(constraints.max_width);
    auto text_size = this->get_size();
    return { text_size.x, text_size.y };
}
void _EmbeddedParagraph::paint(NotNull<PaintingContext*> context, Offsetf offset)
{
    build();
    {
        godot::Color                     p_color    = { 1, 0, 1, 1 };
        godot::Color                     p_dc_color = { 1.f, 1.f, 1.f };
        godot::TextServer::TextDrawProxy proxy      = { context->canvas() };

        auto canvas = context->canvas();
        {
            auto _ = canvas->paint_scope();
            _draw(&proxy, { offset.x, offset.y }, p_color, p_dc_color);
        }
    }
}
void _EmbeddedParagraph::_draw(godot::TextServer::TextDrawProxy* proxy, const skr_float2_t& p_pos, const godot::Color& p_color, const godot::Color& p_dc_color)
{
    const float line_height_scale = 1.f;
    const int   spacing_top       = 0;
    const int   spacing_bottom    = 0;
    godot::RID  p_canvas          = godot::RID::from_uint64((uint64_t)proxy);

    _shape_lines();
    godot::Vector2 ofs      = { p_pos.x, p_pos.y };
    float          h_offset = 0.f;
    if (TS->shaped_text_get_orientation(dropcap_rid) == godot::TextServer::ORIENTATION_HORIZONTAL)
    {
        h_offset = TS->shaped_text_get_size(dropcap_rid).x + dropcap_margins.size.x + dropcap_margins.position.x;
    }
    else
    {
        h_offset = TS->shaped_text_get_size(dropcap_rid).y + dropcap_margins.size.y + dropcap_margins.position.y;
    }

    if (h_offset > 0)
    {
        // Draw dropcap.
        godot::Vector2 dc_off = { ofs.x, ofs.y };
        if (TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_RTL)
        {
            if (TS->shaped_text_get_orientation(dropcap_rid) == godot::TextServer::ORIENTATION_HORIZONTAL)
            {
                dc_off.x += (float)width - h_offset;
            }
            else
            {
                dc_off.y += (float)width - h_offset;
            }
        }
        godot::Vector2 p_pos = { dc_off.x, dc_off.y };
        p_pos.y += TS->shaped_text_get_ascent(dropcap_rid) + dropcap_margins.size.y + dropcap_margins.position.y / 2;
        TS->shaped_text_draw(dropcap_rid, p_canvas, p_pos, -1, -1, p_dc_color);
    }

    int lines_visible = (max_lines_visible >= 0) ? MIN(max_lines_visible, lines_rid.size()) : lines_rid.size();

    for (int i = 0; i < lines_visible; i++)
    {
        float l_width = (float)width;
        if (TS->shaped_text_get_orientation(lines_rid[i]) == godot::TextServer::ORIENTATION_HORIZONTAL)
        {
            ofs.x = p_pos.x;
            ofs.y += TS->shaped_text_get_ascent(lines_rid[i]) * line_height_scale + spacing_top;
            if (i < dropcap_lines)
            {
                if (TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_LTR ||
                    TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_AUTO)
                {
                    ofs.x += h_offset;
                }
                l_width -= h_offset;
            }
        }
        else
        {
            ofs.y = p_pos.y;
            ofs.x += TS->shaped_text_get_ascent(lines_rid[i]) * line_height_scale + spacing_top;
            if (i < dropcap_lines)
            {
                if (TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_LTR ||
                    TS->shaped_text_get_direction(dropcap_rid) == godot::TextServer::DIRECTION_AUTO)
                {
                    ofs.x += h_offset;
                }
                l_width -= h_offset;
            }
        }
        float line_width = TS->shaped_text_get_width(lines_rid[i]);
        if (width > 0)
        {
            float offset = 0.f;
            switch (alignment)
            {
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

            if (TS->shaped_text_get_orientation(lines_rid[i]) == godot::TextServer::ORIENTATION_HORIZONTAL)
            {
                ofs.x += offset;
            }
            else
            {
                ofs.y += offset;
            }
        }
        float clip_l;
        if (TS->shaped_text_get_orientation(lines_rid[i]) == godot::TextServer::ORIENTATION_HORIZONTAL)
        {
            clip_l = MAX(0, p_pos.x - ofs.x);
        }
        else
        {
            clip_l = MAX(0, p_pos.y - ofs.y);
        }
        TS->shaped_text_draw(lines_rid[i], p_canvas, ofs, clip_l, clip_l + l_width, p_color);
        if (TS->shaped_text_get_orientation(lines_rid[i]) == godot::TextServer::ORIENTATION_HORIZONTAL)
        {
            ofs.x = p_pos.x;
            ofs.y += TS->shaped_text_get_descent(lines_rid[i]) * line_height_scale + spacing_bottom;
        }
        else
        {
            ofs.y = p_pos.y;
            ofs.x += TS->shaped_text_get_descent(lines_rid[i]) * line_height_scale + spacing_bottom;
        }
    }
}
} // namespace skr::gui

// old code
// godot::InlineAlignment GetInlineAlignment(EInlineAlignment o)
// {
//     switch (o)
//     {
//         case EInlineAlignment::Baseline:
//             return godot::INLINE_ALIGNMENT_BOTTOM;
//         case EInlineAlignment::Top:
//             return godot::INLINE_ALIGNMENT_TOP;
//         case EInlineAlignment::Middle:
//             return godot::INLINE_ALIGNMENT_CENTER;
//     }
//     return godot::INLINE_ALIGNMENT_BOTTOM;
// }
//
// void RenderText::buildParagraphRec(Paragraph* p, const StyleText& txt)
// {
//     for (auto& inl : inlines_)
//     {
//         std::visit(overloaded{
//                    [&](skr::String& text) {
//                        godot::Color color(txt.color.x, txt.color.y, txt.color.z, txt.color.w);
//                        /*
//                        godot::Color decorationColor(txt.textDecorationColor.X, txt.textDecorationColor.Y, txt.textDecorationColor.Z, txt.textDecorationColor.W);
//                        godot::TextDecorationData decoration;
//                        decoration.decorationColor = decorationColor;
//                        decoration.decorationTexture = nullptr;
//                        decoration.decorationLineFlag = (int64_t)txt.textDecorationLine;
//                        decoration.decorationThickness = txt.textDecorationThickness;
//                        int64_t flags = 0;
//                        */
//                        auto font = static_pointer_cast<godot::Font>(font_);
//                        auto ft = godot::Ref<godot::Font>(font);
//                        p->add_string(godot::String::utf8(text.c_str()), ft, txt.font_size, "", {});
//                    },
//                    [&](RenderObject*& child) {
//                        // if(!child->Visible()) return;
//                        // auto& pos = StylePosition::Get(child->_style);
//                        const auto TODO = EInlineAlignment::Middle;
//                        p->add_object(child, { 0, 0 }, GetInlineAlignment(TODO));
//                    },
//                    [&](RenderText*& child) {
//                        // if(!child->Visible()) return;
//                        // StyleText& ctxt = StyleText::Get(child->_style);
//                        StyleText& ctxt = TODO_StyleText;
//                        child->buildParagraphRec(p, ctxt);
//                    },
//                    [&](skr::SP<BindText>& Bind) {
//                        godot::Color color(txt.color.x, txt.color.y, txt.color.z, txt.color.w);
//                        /*
//                        godot::Color decorationColor(txt.textDecorationColor.X, txt.textDecorationColor.Y, txt.textDecorationColor.Z, txt.textDecorationColor.W);
//                        godot::TextDecorationData decoration;
//                        decoration.decorationColor = decorationColor;
//                        decoration.decorationTexture = nullptr;
//                        decoration.decorationLineFlag = (int64_t)txt.textDecorationLine;
//                        decoration.decorationThickness = txt.textDecorationThickness;
//                        */
//                        auto font = static_pointer_cast<godot::Font>(font_);
//                        auto ft = godot::Ref<godot::Font>(font);
//                        paragraph_->add_string((wchar_t*)Bind->text.c_str(), ft, txt.font_size, "", {});
//                    } },
//                    inl);
//     }
// }