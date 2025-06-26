#pragma once
#include "SkrBase/misc/hash.h" // IWYU pragma: export
#include "SkrGraphics/api.h"

CGPU_EXTERN_C_BEGIN

struct CGPURuntimeTable* cgpu_create_runtime_table();
void cgpu_early_free_runtime_table(struct CGPURuntimeTable* table);
void cgpu_free_runtime_table(struct CGPURuntimeTable* table);
void cgpu_runtime_table_add_queue(CGPUQueueId queue, ECGPUQueueType type, uint32_t index);
CGPUQueueId cgpu_runtime_table_try_get_queue(CGPUDeviceId device, ECGPUQueueType type, uint32_t index);

void cgpu_runtime_table_add_custom_data(struct CGPURuntimeTable* table, const char8_t* key, void* data);
void cgpu_runtime_table_add_sweep_callback(struct CGPURuntimeTable* table, const char8_t* key, void(pfn)(void*), void* usrdata);
void cgpu_runtime_table_add_early_sweep_callback(struct CGPURuntimeTable* table, const char8_t* key, void(pfn)(void*), void* usrdata);
void* cgpu_runtime_table_try_get_custom_data(struct CGPURuntimeTable* table, const char8_t* key);
bool cgpu_runtime_table_remove_custom_data(struct CGPURuntimeTable* table, const char8_t* key);

void CGPUUtil_InitRSParamTables(CGPURootSignature* RS, const struct CGPURootSignatureDescriptor* desc);
void CGPUUtil_FreeRSParamTables(CGPURootSignature* RS);

// check for slot-overlapping and try get a signature from pool
CGPURootSignaturePoolId CGPUUtil_CreateRootSignaturePool(const CGPURootSignaturePoolDescriptor* desc);
CGPURootSignatureId CGPUUtil_TryAllocateSignature(CGPURootSignaturePoolId pool, CGPURootSignature* RSTables, const struct CGPURootSignatureDescriptor* desc);
CGPURootSignatureId CGPUUtil_AddSignature(CGPURootSignaturePoolId pool, CGPURootSignature* sig, const CGPURootSignatureDescriptor* desc);
// TODO: signature pool statics
// void CGPUUtil_AllSignatures(CGPURootSignaturePoolId pool, CGPURootSignatureId* signatures, uint32_t* count);
bool CGPUUtil_PoolFreeSignature(CGPURootSignaturePoolId pool, CGPURootSignatureId sig);
void CGPUUtil_FreeRootSignaturePool(CGPURootSignaturePoolId pool);

#define cgpu_round_up(value, multiple) ((((value) + (multiple)-1) / (multiple)) * (multiple))
#define cgpu_round_down(value, multiple) ((value) - (value) % (multiple))

CGPU_EXTERN_C_END