#pragma once
#include "cgpu/api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CGpuRuntimeTable* cgpu_create_runtime_table();
void cgpu_free_runtime_table(struct CGpuRuntimeTable* table);
void cgpu_runtime_table_add_queue(CGpuQueueId queue, ECGpuQueueType type, uint32_t index);
CGpuQueueId cgpu_runtime_table_try_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);

// TODO: remove this
void CGpuUtil_InitRSParamTables(CGpuRootSignature* RS, const struct CGpuRootSignatureDescriptor* desc);
void CGpuUtil_FreeRSParamTables(CGpuRootSignature* RS);

#ifdef __cplusplus
} // end extern "C"
#endif

#include "utils/hash.h"
#define cgpu_hash(buffer, size, seed) skr_hash((buffer), (size), (seed))
#include "utils/log.h"
#define cgpu_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_warn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)