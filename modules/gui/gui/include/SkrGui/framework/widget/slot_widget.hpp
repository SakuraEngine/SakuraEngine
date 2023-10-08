#pragma once
#include "SkrGui/framework/widget/proxy_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/slot_widget.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "7a561ecb-03dc-4bc4-8577-ff51ce4469f2"
)
SKR_GUI_API SlotWidget : public ProxyWidget {
    SKR_RTTR_GENERATE_BODY()

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    virtual void apply_slot_data(NotNull<RenderObject*> parent, NotNull<RenderObject*> child) const SKR_NOEXCEPT = 0;
};
} // namespace gui sreflect
} // namespace skr sreflect