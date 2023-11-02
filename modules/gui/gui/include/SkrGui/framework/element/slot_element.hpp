#pragma once
#include "SkrGui/framework/element/proxy_element.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/slot_element.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "cddf8085-4ab4-46be-8de7-34bd1a2c290e",
    "rtti": true
)
SlotElement : public ProxyElement
{
    SKR_RTTR_GENERATE_BODY()
    using Super = ProxyElement;
    using Super::Super;

    void updated(NotNull<ProxyWidget*> old_widget) override;
};
} // namespace gui sreflect
} // namespace skr sreflect