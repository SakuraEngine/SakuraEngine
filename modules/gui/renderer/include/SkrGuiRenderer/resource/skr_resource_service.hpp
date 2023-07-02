#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/resource/resource.hpp"
#include "cgpu/io.h"
#include "async/thread_job.hpp"

namespace skr::gui
{
struct SKR_GUI_RENDERER_API SkrResourceService final : public IResourceService {
    SKR_GUI_OBJECT(SkrResourceService, "5be84757-282b-4a1c-8c5e-4b4d58807d7d", IResourceService)

    NotNull<IUpdatableImage*> create_updatable_image(const UpdatableImageDesc& desc) override;
    void                      destroy_resource(NotNull<IResource*> resource) override;

    // getter
    inline skr_io_ram_service_t*  ram_service() const SKR_NOEXCEPT { return _ram_service; }
    inline skr_io_vram_service_t* vram_service() const SKR_NOEXCEPT { return _vram_service; }
    inline skr_vfs_t*             vfs() const SKR_NOEXCEPT { return _vfs; }
    inline FutureLauncher<bool>*  future_launcher() const SKR_NOEXCEPT { return _future_launcher; }

private:
    skr_io_ram_service_t*  _ram_service;
    skr_io_vram_service_t* _vram_service;
    skr_vfs_t*             _vfs;
    FutureLauncher<bool>*  _future_launcher;
};
} // namespace skr::gui