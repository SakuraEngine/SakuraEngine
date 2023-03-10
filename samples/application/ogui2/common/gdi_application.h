#pragma once
#include "../../../common/render_application.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIRenderer, skr_gdi_renderer)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, SGDIDevice, skr_gdi_device)

struct gdi_application_t
{
    render_application_t gfx;
    struct skr_vfs_t* resource_vfs SKR_IF_CPP(= nullptr);
    struct skr_io_ram_service_t* ram_service SKR_IF_CPP(= nullptr);
    struct skr_io_vram_service_t* vram_service SKR_IF_CPP(= nullptr);
    struct skr_threaded_service_t* aux_service SKR_IF_CPP(= nullptr);
    skr_gdi_renderer_id renderer = nullptr;
    skr_gdi_device_id device = nullptr;
};

bool initialize_gdi_application(gdi_application_t* app);
bool finalize_gdi_application(gdi_application_t* app);