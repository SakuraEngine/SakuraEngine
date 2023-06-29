#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/embed_services.hpp"

namespace skr::gui
{
struct SkrRenderDevice;
struct SKR_GUI_RENDERER_API SkrResourceDevice final : public IEmbeddedTextServiceResourceProvider {
    void init(SkrRenderDevice* render_device);
    void shutdown();

    NotNull<IUpdatableImageEntry*> create_updatable_image_entry() override;
    void                           destroy_updatable_image_entry(NotNull<IUpdatableImageEntry*> entry) override;
    Array<uint8_t>                 read_font_file(StringView path) override;

private:
    SkrRenderDevice* _render_device = nullptr;
};
} // namespace skr::gui