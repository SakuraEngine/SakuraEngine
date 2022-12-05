#pragma once
#include "api.h"

typedef const char* CGPUXName;
DEFINE_CGPU_OBJECT(CGPUXBindTable)
struct CGPUXBindTableDescriptor;

RUNTIME_EXTERN_C RUNTIME_API
CGPUXBindTableId cgpux_create_bind_table(CGPUDeviceId device, const struct CGPUXBindTableDescriptor* desc);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_bind_table_update(CGPUXBindTableId table, const struct CGPUDescriptorData* datas, uint32_t count);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_bind_table_override(CGPUXBindTableId table, CGPUXBindTableId another);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_free_bind_table(CGPUXBindTableId bind_table);

typedef struct CGPUXBindTableDescriptor {
    CGPURootSignatureId root_signature;
    const CGPUXName* names;
    uint32_t names_count;
} CGPUXBindTableDescriptor;
