#pragma once
#include "SkrGui/framework/layer/layer.hpp"
#include "SkrGui/framework/layer/container_layer.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "ea23bfe8-33db-4277-a415-bf059bf76f46"
)]] SKR_GUI_API ContainerLayer : public Layer
{
    SKR_GENERATE_BODY(ContainerLayer)

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    void add_child(NotNull<Layer*> child) SKR_NOEXCEPT;
    bool has_children() const SKR_NOEXCEPT;
    void remove_all_children() SKR_NOEXCEPT;

    inline Span<Layer* const> children() const SKR_NOEXCEPT { return { _children.data(), _children.size() }; }

private:
    Array<Layer*> _children;
};
} // namespace skr::gui