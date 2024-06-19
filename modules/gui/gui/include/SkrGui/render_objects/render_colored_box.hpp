#pragma once
#include "SkrGui/framework/render_object/render_proxy_box_with_hit_test_behavior.hpp"
#include "SkrGui/math/color.hpp"
#ifndef __meta__
    #include "SkrGui/render_objects/render_colored_box.generated.h"
#endif

namespace skr::gui
{
sreflect_struct(
    "guid": "02cc61fb-9ca4-464b-95a5-2a5ad277abf8"
)
RenderColoredBox : public RenderProxyBoxWithHitTestBehavior {
    SKR_RTTR_GENERATE_BODY()
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