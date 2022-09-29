#pragma once
#include "SkrRenderer/skr_renderer.configure.h"

#ifdef __cplusplus
namespace skr { struct RendererDevice; }
typedef struct skr::RendererDevice SRenderDevice;

class SkrRendererModule;
#else
typedef struct SRenderDevice SRenderDevice;
#endif
typedef SRenderDevice* SRenderDeviceId;
