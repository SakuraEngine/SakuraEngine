#pragma once
#include "SkrGui/framework/render_object/render_proxy_box.hpp"
#include "SkrGui/math/color.hpp"

namespace skr::gui
{
struct RenderColoredBox : public RenderProxyBox {
    SKR_GUI_OBJECT(RenderColoredBox, "ffe5f08b-8d7c-4d49-a9f5-37565bdebe32", RenderProxyBox)

    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

    // getter setter
    inline const Color& color() const SKR_NOEXCEPT { return _color; }
    inline void         set_color(const Color& color) SKR_NOEXCEPT
    {
        if (_color != color)
        {
            _color = color;
            mark_needs_paint();
        }
    }

private:
    Color _color = {};
};
} // namespace skr::gui