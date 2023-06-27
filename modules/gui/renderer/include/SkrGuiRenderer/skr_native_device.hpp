#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/device.hpp"
#include "SkrGui/backend/resource/resource_provider.hpp"

namespace skr::gui
{
struct SKR_GUI_RENDERER_API SkrNativeDevice : public INativeDevice, public IResourceProvider {
    SKR_GUI_OBJECT(SkrNativeDevice, "27cf3f49-efda-4ae8-b5e6-89fb3bb0590e", INativeDevice, IResourceProvider)

    //==> Begin IResourceProvider
    bool                     support_entry(SKR_GUI_TYPE_ID entry_type) const SKR_NOEXCEPT override;
    NotNull<IResourceEntry*> create_entry(SKR_GUI_TYPE_ID entry_type, const void* create_data) override;
    void                     destroy_entry(NotNull<IResourceEntry*> entry) SKR_NOEXCEPT override;
    //==> End IResourceProvider
};
} // namespace skr::gui
