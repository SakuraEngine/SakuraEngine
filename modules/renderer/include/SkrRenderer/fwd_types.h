#pragma once
#include "SkrRenderer/module.configure.h"
#include "utils/types.h"

#ifdef __cplusplus
namespace skr { struct RendererDevice; }
struct skr_threaded_service_t;
namespace skr { namespace io { class VRAMService; } }
typedef struct skr::RendererDevice SRenderDevice;

class SkrRendererModule;
#else
typedef struct SRenderDevice SRenderDevice;
#endif
typedef SRenderDevice* SRenderDeviceId;

typedef skr_guid_t skr_vertex_layout_id;