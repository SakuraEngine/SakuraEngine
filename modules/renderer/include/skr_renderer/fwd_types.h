#pragma once
#include "SkrRenderer/module.configure.h"

#ifdef __cplusplus
namespace skr { struct RendererDevice; }
typedef struct skr::RendererDevice SRenderDevice;

class SkrRendererModule;
#else
typedef struct SRenderDevice SRenderDevice;
#endif
typedef SRenderDevice* SRenderDeviceId;
