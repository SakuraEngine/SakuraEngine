#pragma once
#include "SkrGui/framework/element/component_element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/element/proxy_element.generated.h"

namespace skr::gui
{

struct [[sattr(guid = "5b6fca8a-7558-4301-a00a-749b63be5aab"
)]] SKR_GUI_API ProxyElement : public ComponentElement
{
    SKR_GENERATE_BODY(ProxyElement)
    SKR_IFNOT_META(using Super::Super);

    // build & update
    void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;

    Widget* build() SKR_NOEXCEPT override;

    virtual void updated(NotNull<ProxyWidget*> old_widget) = 0;
};
} // namespace skr::gui