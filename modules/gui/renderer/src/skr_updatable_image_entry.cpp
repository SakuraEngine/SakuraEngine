#include "SkrGuiRenderer/skr_updatable_image_entry.hpp"
#include "SkrGuiRenderer/skr_native_device.hpp"

namespace skr::gui
{
NotNull<IResourceProvider*> SkrUpdatableImageEntry::provider() const SKR_NOEXCEPT
{
    return make_not_null(_provider);
}
void SkrUpdatableImageEntry::destroy_resoure(NotNull<IResource*> resource) SKR_NOEXCEPT
{
    SkrDelete(resource.get());
}
void SkrUpdatableImageEntry::visit_requested_resources(VisitResourceFuncRef visitor)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
NotNull<IImage*> SkrUpdatableImageEntry::request_image_of_size(Sizef show_size) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return make_not_null<IImage*>(nullptr);
}
void SkrUpdatableImageEntry::visit_useable_image_size(FunctionRef<void(Sizef)> visitor) const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void SkrUpdatableImageEntry::update(const ImageUpdateDesc& desc) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
} // namespace skr::gui