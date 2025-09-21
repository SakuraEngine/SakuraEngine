#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/widget/proxy_widget.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "3b3208fe-f5df-419d-840c-6621dc1661d2"
)]] SKR_GUI_API ProxyWidget : public Widget
{
    SKR_GENERATE_BODY(ProxyWidget)

    Widget* child = nullptr;
};
} // namespace skr::gui