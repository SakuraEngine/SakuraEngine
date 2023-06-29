#include "SkrGuiRenderer/resource/skr_resource_device.hpp"
#include "SkrGuiRenderer/resource/skr_updatable_image_entry.hpp"

namespace skr::gui
{
void SkrResourceDevice::init(SkrRenderDevice* render_device)
{
    _render_device = render_device;
}
void SkrResourceDevice::shutdown()
{
    _render_device = nullptr;
}

NotNull<IUpdatableImageEntry*> SkrResourceDevice::create_updatable_image_entry()
{
    return make_not_null(SkrNew<SkrUpdatableImageEntry>(this, _render_device));
}
void SkrResourceDevice::destroy_updatable_image_entry(NotNull<IUpdatableImageEntry*> entry)
{
    SkrDelete(entry.get());
}
Array<uint8_t> SkrResourceDevice::read_font_file(StringView path)
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return {};
}
} // namespace skr::gui