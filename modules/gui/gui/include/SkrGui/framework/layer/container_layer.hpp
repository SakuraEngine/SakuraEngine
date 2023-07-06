#pragma once
#include "SkrGui/framework/layer/layer.hpp"

namespace skr::gui
{
struct ContainerLayer : public Layer {
    SKR_GUI_OBJECT(ContainerLayer, "bb5a6b46-30b2-49f3-b445-260f9372bbb5", Layer)

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    void add_child(NotNull<Layer*> child) SKR_NOEXCEPT;
    bool has_children() const SKR_NOEXCEPT;
    void remove_all_children() SKR_NOEXCEPT;

private:
    Array<Layer*> _children;
};
} // namespace skr::gui