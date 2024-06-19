#pragma once
#include "SkrGui/framework/layer/layer.hpp"
#ifndef __meta__
    #include "SkrGui/framework/layer/container_layer.generated.h"
#endif

namespace skr::gui
{
sreflect_struct(
    "guid": "ea23bfe8-33db-4277-a415-bf059bf76f46"
)
SKR_GUI_API ContainerLayer : public Layer {
    SKR_RTTR_GENERATE_BODY()

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    void add_child(NotNull<Layer*> child) SKR_NOEXCEPT;
    bool has_children() const SKR_NOEXCEPT;
    void remove_all_children() SKR_NOEXCEPT;

    inline Span<Layer* const> children() const SKR_NOEXCEPT { return { _children.data(), _children.size() }; }

private:
    Array<Layer*> _children;
};
} // namespace skr::gui