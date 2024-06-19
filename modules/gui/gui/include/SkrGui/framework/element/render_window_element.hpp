#pragma once
#include "render_object_element.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/render_window_element.generated.h"
#endif

namespace skr::gui
{

sreflect_struct(
    "guid": "68cea8f8-f39c-41e3-90dc-615308fb8034"
)
SKR_GUI_API RenderWindowElement : public RenderObjectElement {
    SKR_RTTR_GENERATE_BODY()
    using Super = RenderObjectElement;
    using Super::Super;
};
} // namespace skr::gui