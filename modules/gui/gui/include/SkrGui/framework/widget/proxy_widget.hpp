#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/proxy_widget.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "3b3208fe-f5df-419d-840c-6621dc1661d2"
)
SKR_GUI_API ProxyWidget : public Widget {
    SKR_RTTR_GENERATE_BODY()

    Widget* child = nullptr;
};
} // namespace gui sreflect
} // namespace skr sreflect