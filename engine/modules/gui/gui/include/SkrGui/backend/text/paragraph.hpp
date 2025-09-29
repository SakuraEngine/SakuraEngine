#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/box_constraint.hpp"
#include "SkrGui/backend/text/paragraph.generated.h"

namespace skr::gui
{
struct TextStyle;
struct PaintingContext;
struct [[sattr(
    guid = "fe659fa3-d171-4d41-a95a-849618c3765b"
)]] SKR_GUI_API IParagraph : virtual public skr::IRTTRBasic
{
    SKR_GENERATE_BODY(IParagraph)
    virtual ~IParagraph() = default;

    virtual void clear() = 0;
    virtual void build() = 0;
    virtual void add_text(const String& text, const TextStyle& style) = 0;
    virtual Sizef layout(BoxConstraints constraints) = 0;
    virtual void paint(NotNull<PaintingContext*> context, Offsetf offset) = 0;
};
} // namespace skr::gui