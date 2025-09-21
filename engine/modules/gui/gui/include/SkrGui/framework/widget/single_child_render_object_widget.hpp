#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/widget/single_child_render_object_widget.generated.h"

namespace skr::gui
{

struct [[sattr(guid = "57df4e45-aefa-49e8-9d5b-0f03b468d0b1"
)]] SKR_GUI_API SingleChildRenderObjectWidget : public RenderObjectWidget
{
    SKR_GENERATE_BODY(SingleChildRenderObjectWidget)

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    Widget* child = nullptr;
};
} // namespace skr::gui