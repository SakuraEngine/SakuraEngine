#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/resource/resource.hpp"
#include "cgpu/io.h"
#include "async/thread_job.hpp"

namespace skr::gui
{
struct SKR_GUI_RENDERER_API SkrResourceDevice final {
    void init();
    void shutdown();

    NotNull<IUpdatableImage*> create_updatable_image(const UpdatableImageDesc& desc);
    void                      destroy_resource(NotNull<IResource*> resource);

    // getter
    inline skr_io_ram_service_t*  ram_service() const SKR_NOEXCEPT { return _ram_service; }
    inline skr_io_vram_service_t* vram_service() const SKR_NOEXCEPT { return _vram_service; }
    inline skr_vfs_t*             vfs() const SKR_NOEXCEPT { return _vfs; }
    inline FutureLauncher<bool>*  future_launcher() const SKR_NOEXCEPT { return _future_launcher; }

private:
    skr_io_ram_service_t*  _ram_service;
    skr_io_vram_service_t* _vram_service;
    skr_vfs_t*             _vfs;
    JobQueue*              _job_queue;
    FutureLauncher<bool>*  _future_launcher;
};
} // namespace skr::gui