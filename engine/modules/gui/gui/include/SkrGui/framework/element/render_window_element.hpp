#pragma once
#include "render_object_element.hpp"
#include "SkrGui/framework/element/render_window_element.generated.h"

namespace skr::gui
{

struct [[sattr(guid = "68cea8f8-f39c-41e3-90dc-615308fb8034"
)]] SKR_GUI_API RenderWindowElement : public RenderObjectElement
{
    SKR_GENERATE_BODY(RenderWindowElement)
    using Super = RenderObjectElement;
    using Super::Super;
};
} // namespace skr::gui