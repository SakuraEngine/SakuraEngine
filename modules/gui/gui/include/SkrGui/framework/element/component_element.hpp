#pragma once
#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/component_element.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "8344aca9-b204-4b4a-8e16-18e4219039be"
)
SKR_GUI_API ComponentElement : public Element {
    SKR_RTTR_GENERATE_BODY()
    using Super = Element;
    using Super::Super;

    // lifecycle & tree
    virtual void first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT override;
    void         visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    // build & update
    void perform_rebuild() SKR_NOEXCEPT override;

    virtual Widget* build() SKR_NOEXCEPT = 0;

private:
    Element* _child;
};
} // namespace gui sreflect
} // namespace skr sreflect