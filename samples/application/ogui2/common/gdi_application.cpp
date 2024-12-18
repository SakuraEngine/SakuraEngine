#include "./gdi_application.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrOS/filesystem.hpp"
#include "SkrCore/platform/vfs.h"
#include "SkrRT/io/ram_io.hpp"
#include "SkrCore/async/thread_job.hpp"

#include "SkrContainers/string.hpp"

// bool initialize_gdi_application(gdi_application_t* app)
// {
//     // initialize gfx
//     app->gfx.backend = platform_default_backend;
//     auto app_name = skr::String::from_utf8(SKR_UTF8("GDI [backend:"));
//     app_name += gCGPUBackendNames[app->gfx.backend];
//     app_name += SKR_UTF8("]");
//     app->gfx.window_title = app_name.u8_str();
//     if (app_create_window(&app->gfx, 900, 900)) return false;
//     if (app_create_gfx_objects(&app->gfx)) return false;

//     // initialize services
//     {
//         std::error_code ec = {};
//         auto            resourceRoot = (skr::filesystem::current_path(ec) / "../resources");
//         auto            u8ResourceRoot = resourceRoot.u8string();
//         skr_vfs_desc_t  vfs_desc = {};
//         vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
//         vfs_desc.override_mount_dir = u8ResourceRoot.c_str();
//         app->resource_vfs = skr_create_vfs(&vfs_desc);
//     }
//     {
//         auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
//         ioServiceDesc.name = SKR_UTF8("GUI-IRAMService");
//         ioServiceDesc.sleep_time = 1000 / 60;
//         app->ram_service = skr_io_ram_service_t::create(&ioServiceDesc);
//         app->ram_service->run();
//     }
//     {
//         auto ioServiceDesc = make_zeroed<skr_vram_io_service_desc_t>();
//         ioServiceDesc.name = SKR_UTF8("GUI-VRAMService");
//         ioServiceDesc.sleep_time = 1000 / 60;
//         ioServiceDesc.lockless = true;
//         app->vram_service = skr_io_vram_service_t::create(&ioServiceDesc);
//     }
//     {
//         auto jqDesc = make_zeroed<skr::JobQueueDesc>();
//         jqDesc.thread_count = 2;
//         jqDesc.priority = SKR_THREAD_NORMAL;
//         jqDesc.name = u8"GDIApp-JobQueue";
//         app->job_queue = SkrNew<skr::JobQueue>(jqDesc);
//     }

//     // initialize gdi device
//     app->device = skr::gui::IGDIDevice::Create(skr::gui::EGDIBackend::NANOVG);

//     // initialize gdi renderer
//     skr::gui::GDIRendererDescriptor             gdir_desc = {};
//     skr::gui::GDIRendererDescriptor_RenderGraph gdir_desc2 = {};
//     gdir_desc2.target_format = (ECGPUFormat)app->gfx.swapchain->back_buffers[0]->info->format;
//     gdir_desc2.device = app->gfx.device;
//     gdir_desc2.transfer_queue = app->gfx.gfx_queue;
//     gdir_desc2.vfs = app->resource_vfs;
//     gdir_desc2.ram_service = app->ram_service;
//     gdir_desc2.vram_service = app->vram_service;
//     gdir_desc2.job_queue = app->job_queue;
//     gdir_desc.usr_data = &gdir_desc2;
//     app->renderer = SkrNew<skr::gui::GDIRenderer_RenderGraph>();
//     app->renderer->initialize(&gdir_desc);

//     // initialize text
//     // skr::gui::IGDIText::Initialize(app->renderer);
//     return true;
// }

// bool finalize_gdi_application(gdi_application_t* app)
// {
//     app_wait_gpu_idle(&app->gfx);
//     skr::gui::IGDIDevice::Free(app->device);

//     // skr::gui::IGDIText::Finalize();
//     app->renderer->finalize();
//     SkrDelete(app->renderer);

//     app_finalize(&app->gfx);

//     if (app->job_queue) SkrDelete(app->job_queue);
//     if (app->vram_service) skr_io_vram_service_t::destroy(app->vram_service);
//     if (app->ram_service) skr_io_ram_service_t::destroy(app->ram_service);
//     if (app->resource_vfs) skr_free_vfs(app->resource_vfs);
//     return true;
// }
