#pragma once
#include "cgpu/backend/metal/cgpu_metal.h"
#include "../common/common_utils.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct CGPUAdapter_Metal CGPUAdapter_Metal;

// Feature Select Helpers
void MetalUtil_EnumFormatSupports(struct CGPUAdapter_Metal* MAdapter);
void MetalUtil_RecordAdapterDetail(struct CGPUAdapter_Metal* MAdapter);

#ifdef __cplusplus
}
#endif