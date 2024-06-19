#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/single_child_render_object_widget.generated.h"
#endif

namespace skr::gui
{

sreflect_struct(
    "guid": "57df4e45-aefa-49e8-9d5b-0f03b468d0b1"
)
SKR_GUI_API SingleChildRenderObjectWidget : public RenderObjectWidget {
    SKR_RTTR_GENERATE_BODY()

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    Widget* child = nullptr;
};
} // namespace skr::gui