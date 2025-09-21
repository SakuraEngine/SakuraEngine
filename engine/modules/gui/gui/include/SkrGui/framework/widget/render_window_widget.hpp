#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"
#include "SkrGui/framework/widget/render_window_widget.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "8a8af20a-df7e-42b2-a4f8-108a1fea2137"
)]] SKR_GUI_API RenderWindowWidget : public RenderObjectWidget
{
    SKR_GENERATE_BODY(RenderWindowWidget)
    Widget* child = nullptr;
};
} // namespace skr::gui