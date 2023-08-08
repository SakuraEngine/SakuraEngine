#pragma once
#include "SkrRT/io/ram_io.hpp"
#include "SkrGuiRenderer/module.configure.h"
#include "SkrRT/async/thread_job.hpp"

namespace skr::gui
{
struct SKR_GUI_RENDERER_API SkrResourceDevice final {
    void init();
    void shutdown();

    // getter
    inline skr_io_ram_service_t* ram_service() const SKR_NOEXCEPT { return _ram_service; }
    inline skr_vfs_t*            vfs() const SKR_NOEXCEPT { return _vfs; }
    inline FutureLauncher<bool>* future_launcher() const SKR_NOEXCEPT { return _future_launcher; }

private:
    skr_io_ram_service_t* _ram_service     = nullptr;
    skr_vfs_t*            _vfs             = nullptr;
    JobQueue*             _job_queue       = nullptr;
    FutureLauncher<bool>* _future_launcher = nullptr;
};
} // namespace skr::gui