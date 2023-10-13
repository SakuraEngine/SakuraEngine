#pragma once
#include "SkrGui/framework/render_object/render_proxy_box.hpp"
#include "SkrGui/math/color.hpp"
#ifndef __meta__
    #include "SkrGui/render_objects/render_colored_box.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "02cc61fb-9ca4-464b-95a5-2a5ad277abf8",
    "rtti": true
)
RenderColoredBox : public RenderProxyBox
{
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
    // TODO. enable field reflection
    spush_attr("no-rtti": true)
    Color _color = {};
};
} // namespace gui sreflect
} // namespace skr sreflect