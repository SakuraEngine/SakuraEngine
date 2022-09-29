#pragma once
#include "SkrRenderer/skr_renderer.configure.h"

#ifdef __cplusplus
namespace skr { struct RendererDevice; }
typedef struct skr::RendererDevice SRenderDevice;
#else
typedef struct SRenderDevice SRenderDevice;
#endif
typedef SRenderDevice* SRenderDeviceId;