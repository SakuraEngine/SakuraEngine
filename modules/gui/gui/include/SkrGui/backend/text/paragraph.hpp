#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/box_constraint.hpp"

namespace skr::gui
{
struct TextStyle;
struct PaintingContext;
struct SKR_GUI_API IParagraph SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IParagraph, "467730e3-29dd-4db1-8a81-4eb7c51ede64")
    virtual ~IParagraph() = default;

    virtual void  clear()                                                  = 0;
    virtual void  build()                                                  = 0;
    virtual void  add_text(const String& text, const TextStyle& style)     = 0;
    virtual Sizef layout(BoxConstraints constraints)                       = 0;
    virtual void  paint(NotNull<PaintingContext*> context, Offsetf offset) = 0;
};
} // namespace skr::gui