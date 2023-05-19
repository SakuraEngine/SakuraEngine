#pragma once
#include "common/render_application.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIRenderer, skr_gdi_renderer)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)

typedef struct gdi_application_t
{
    render_application_t gfx;
    struct skr_vfs_t* resource_vfs SKR_IF_CPP(= nullptr);
    struct skr_io_ram_service_t* ram_service SKR_IF_CPP(= nullptr);
    struct skr_io_vram_service_t* vram_service SKR_IF_CPP(= nullptr);
    skr_job_queue_id job_queue SKR_IF_CPP(= nullptr);
    skr_gdi_renderer_id renderer SKR_IF_CPP(= nullptr);
    skr_gdi_device_id device SKR_IF_CPP(= nullptr);
} gdi_application_t;

bool initialize_gdi_application(gdi_application_t* app);
bool finalize_gdi_application(gdi_application_t* app);