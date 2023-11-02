#pragma once
#include "SkrGui/framework/layer/layer.hpp"
#ifndef __meta__
    #include "SkrGui/framework/layer/geometry_layer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "1d1fbcab-eb50-4a22-99f6-59c5f4aca3e9",
    "rtti": true
)
GeometryLayer : public Layer
{
    SKR_RTTR_GENERATE_BODY()
    using Super = Layer;

    // lifecycle & tree
    // ctor -> mount <-> unmount -> destroy
    void attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    inline ICanvas* canvas() const SKR_NOEXCEPT { return _canvas; }

private:
    // TODO. enable field reflection
    spush_attr("no-rtti": true)
    ICanvas* _canvas = nullptr;
};
} // namespace gui sreflect
} // namespace skr sreflect