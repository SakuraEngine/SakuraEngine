#include "SkrGui/framework/layer/geometry_layer.hpp"
#include "SkrGui/backend/device/device.hpp"
#include "SkrGui/framework/build_owner.hpp"

namespace skr::gui
{
void GeometryLayer::attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT
{
    Super::attach(owner);
    _canvas = owner->native_device()->create_canvas();
}
void GeometryLayer::visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT
{
}

} // namespace skr::gui