#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/box_constraint.hpp"
#ifndef __meta__
    #include "SkrGui/backend/text/paragraph.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct TextStyle;
struct PaintingContext;
sreflect_struct(
    "guid": "fe659fa3-d171-4d41-a95a-849618c3765b",
    "rtti": true
)
SKR_GUI_API IParagraph : virtual public skr::rttr::IObject
{
    SKR_RTTR_GENERATE_BODY()
    virtual ~IParagraph() = default;

    virtual void  clear()                                                  = 0;
    virtual void  build()                                                  = 0;
    virtual void  add_text(const String& text, const TextStyle& style)     = 0;
    virtual Sizef layout(BoxConstraints constraints)                       = 0;
    virtual void  paint(NotNull<PaintingContext*> context, Offsetf offset) = 0;
};
} // namespace gui sreflect
} // namespace skr sreflect