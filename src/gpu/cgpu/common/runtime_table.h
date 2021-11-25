#pragma once
#include "cgpu/api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CGpuRuntimeTable* cgpu_create_runtime_table();
void cgpu_free_runtime_table(struct CGpuRuntimeTable* table);
void cgpu_runtime_table_add_queue(CGpuQueueId queue, ECGpuQueueType type, uint32_t index);
CGpuQueueId cgpu_runtime_table_try_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);

#ifdef __cplusplus
} // end extern "C"
#endif
