#pragma once
#include "../../../common/render_application.h"

#ifdef __cplusplus
namespace skr { namespace gdi { struct SGDIRenderer; struct SGDIDevice; } }
#endif

struct gdi_application_t
{
    render_application_t gfx;
    struct skr_vfs_t* resource_vfs SKR_IF_CPP(= nullptr);
    struct skr_io_ram_service_t* ram_service SKR_IF_CPP(= nullptr);
    struct skr_io_vram_service_t* vram_service SKR_IF_CPP(= nullptr);
    struct skr_threaded_service_t* aux_service SKR_IF_CPP(= nullptr);
#ifdef __cplusplus
    skr::gdi::SGDIRenderer* renderer = nullptr;
    skr::gdi::SGDIDevice* device = nullptr;
#else
    void* render;
    void* device;
#endif
};

bool initialize_gdi_application(gdi_application_t* app);
bool finalize_gdi_application(gdi_application_t* app);