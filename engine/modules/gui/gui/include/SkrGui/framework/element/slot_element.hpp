#pragma once
#include "SkrGui/framework/element/proxy_element.hpp"
#include "SkrGui/framework/element/slot_element.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "cddf8085-4ab4-46be-8de7-34bd1a2c290e")]]
SlotElement : public ProxyElement
{
    SKR_GENERATE_BODY(SlotElement)
    SKR_IFNOT_META(using Super::Super);

    void updated(NotNull<ProxyWidget*> old_widget) override;
};
} // namespace skr::gui