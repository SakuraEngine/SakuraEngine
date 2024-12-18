#include "SkrGuiRenderer/resource/skr_resource_device.hpp"
#include "SkrCore/platform/vfs.h"
#include "SkrOS/filesystem.hpp"
#include "SkrBase/misc/make_zeroed.hpp"

namespace skr::gui
{
void SkrResourceDevice::init()
{
    // initialize services
    {
        std::error_code ec             = {};
        auto            resourceRoot   = (skr::filesystem::current_path(ec) / "../resources");
        auto            u8ResourceRoot = resourceRoot.u8string();
        skr_vfs_desc_t  vfs_desc       = {};
        vfs_desc.mount_type            = SKR_MOUNT_TYPE_CONTENT;
        vfs_desc.override_mount_dir    = u8ResourceRoot.c_str();
        _vfs                           = skr_create_vfs(&vfs_desc);
    }
    {
        auto ioServiceDesc       = make_zeroed<skr_ram_io_service_desc_t>();
        ioServiceDesc.name       = SKR_UTF8("GUI-IRAMService");
        ioServiceDesc.sleep_time = 1000 / 60;
        _ram_service             = skr_io_ram_service_t::create(&ioServiceDesc);
        _ram_service->run();
    }
    {
        auto jqDesc         = make_zeroed<skr::JobQueueDesc>();
        jqDesc.thread_count = 2;
        jqDesc.priority     = SKR_THREAD_NORMAL;
        jqDesc.name         = u8"GDIApp-JobQueue";
        _job_queue          = SkrNew<skr::JobQueue>(jqDesc);
        _future_launcher    = SkrNew<FutureLauncher<bool>>(_job_queue);
    }
}
void SkrResourceDevice::shutdown()
{
    if (_future_launcher) SkrDelete(_future_launcher);
    if (_job_queue) SkrDelete(_job_queue);
    if (_ram_service) skr_io_ram_service_t::destroy(_ram_service);
    if (_vfs) skr_free_vfs(_vfs);
}
} // namespace skr::gui