#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/render_window_widget.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "8a8af20a-df7e-42b2-a4f8-108a1fea2137",
    "rtti": true
)
SKR_GUI_API RenderWindowWidget : public RenderObjectWidget
{
    SKR_RTTR_GENERATE_BODY()
    Widget* child = nullptr;
};
} // namespace gui sreflect
} // namespace skr sreflect