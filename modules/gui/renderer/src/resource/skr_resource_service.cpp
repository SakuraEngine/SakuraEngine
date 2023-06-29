#include "SkrGuiRenderer/resource/skr_resource_service.hpp"

namespace skr::gui
{
NotNull<IUpdatableImage*> SkrResourceService::create_updatable_image(const UpdatableImageDesc& desc)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return make_not_null<IUpdatableImage*>(nullptr);
}
void SkrResourceService::destroy_resource(NotNull<IResource*> resource)
{
    SkrDelete(resource.get());
}
} // namespace skr::gui