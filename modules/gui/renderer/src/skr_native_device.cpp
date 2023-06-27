#include "SkrGuiRenderer/skr_native_device.hpp"
#include "SkrGuiRenderer/skr_updatable_image_entry.hpp"

namespace skr::gui
{

//==> Begin IResourceProvider
bool SkrNativeDevice::support_entry(SKR_GUI_TYPE_ID entry_type) const SKR_NOEXCEPT
{
    if (entry_type == SKR_GUI_TYPE_ID_OF_STATIC(IUpdatableImageEntry))
    {
        return true;
    }
    return false;
}
NotNull<IResourceEntry*> SkrNativeDevice::create_entry(SKR_GUI_TYPE_ID entry_type, const void* create_data)
{
    if (entry_type == SKR_GUI_TYPE_ID_OF_STATIC(IUpdatableImageEntry))
    {
        return make_not_null<IResourceEntry*>(SkrNew<SkrUpdatableImageEntry>());
    }

    // Toggle error
    return make_not_null<IResourceEntry*>(nullptr);
}
void SkrNativeDevice::destroy_entry(NotNull<IResourceEntry*> entry) SKR_NOEXCEPT
{
    SkrDelete(entry.get());
}
//==> End IResourceProvider
} // namespace skr::gui