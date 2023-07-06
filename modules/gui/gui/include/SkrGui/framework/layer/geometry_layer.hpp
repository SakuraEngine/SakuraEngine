#pragma once
#include "SkrGui/framework/layer/layer.hpp"

namespace skr::gui
{
struct GeometryLayer : public Layer {
    SKR_GUI_OBJECT(GeometryLayer, "8594695b-da35-462a-b4bf-b4ab891088c5", Layer)
    using Super = Layer;

    // lifecycle & tree
    // ctor -> mount <-> unmount -> destroy
    void attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    inline ICanvas* canvas() const SKR_NOEXCEPT { return _canvas; }

private:
    ICanvas* _canvas = nullptr;
};
} // namespace skr::gui