#pragma once
#include "render_object_element.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderWindowElement : public RenderObjectElement {
    SKR_GUI_OBJECT(RenderWindowElement, "81b17af7-7a01-41f4-863b-4de0a1488a44", RenderObjectElement)
    using Super = RenderObjectElement;
    using Super::Super;
};
} // namespace skr::gui