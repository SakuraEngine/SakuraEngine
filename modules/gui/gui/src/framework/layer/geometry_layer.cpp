#include "SkrGui/framework/layer/geometry_layer.hpp"
#include "SkrGui/framework/pipeline_owner.hpp"
#include "SkrGui/backend/device/device.hpp"

namespace skr::gui
{
void GeometryLayer::attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT
{
    Super::attach(owner);
    _canvas = owner->native_device()->create_canvas();
}
void GeometryLayer::visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT
{
}

} // namespace skr::gui