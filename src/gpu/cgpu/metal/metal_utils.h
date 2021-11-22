#pragma once
#include "cgpu/backend/metal/cgpu_metal.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct CGpuAdapter_Metal CGpuAdapter_Metal;

// Feature Select Helpers
void MetalUtil_EnumFormatSupports(struct CGpuAdapter_Metal* MAdapter);
void MetalUtil_RecordAdapterDetail(struct CGpuAdapter_Metal* MAdapter);

#ifdef __cplusplus
}
#endif