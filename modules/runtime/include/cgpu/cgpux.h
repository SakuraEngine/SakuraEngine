#pragma once
#include "cgpu/api.h"

typedef const char8_t* CGPUXName;
DEFINE_CGPU_OBJECT(CGPUXBindTable)
DEFINE_CGPU_OBJECT(CGPUXMergedBindTable)
struct CGPUXBindTableDescriptor;
struct CGPUXMergedBindTableDescriptor;

RUNTIME_EXTERN_C RUNTIME_API
CGPUXBindTableId cgpux_create_bind_table(CGPUDeviceId device, const struct CGPUXBindTableDescriptor* desc);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_bind_table_update(CGPUXBindTableId table, const struct CGPUDescriptorData* datas, uint32_t count);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_render_encoder_bind_bind_table(CGPURenderPassEncoderId encoder, CGPUXBindTableId table);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_compute_encoder_bind_bind_table(CGPUComputePassEncoderId encoder, CGPUXBindTableId table);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_free_bind_table(CGPUXBindTableId bind_table);

RUNTIME_EXTERN_C RUNTIME_API
CGPUXMergedBindTableId cgpux_create_megred_bind_table(CGPUDeviceId device, const struct CGPUXMergedBindTableDescriptor* desc);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_merged_bind_table_merge(CGPUXMergedBindTableId table, const CGPUXBindTableId* tables, uint32_t count);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_render_encoder_bind_merged_bind_table(CGPURenderPassEncoderId encoder, CGPUXMergedBindTableId table);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_compute_encoder_bind_merged_bind_table(CGPUComputePassEncoderId encoder, CGPUXMergedBindTableId table);

RUNTIME_EXTERN_C RUNTIME_API
void cgpux_free_merged_bind_table(CGPUXMergedBindTableId merged_table);

typedef struct CGPUXBindTableDescriptor {
    CGPURootSignatureId root_signature;
    const CGPUXName* names;
    uint32_t names_count;
} CGPUXBindTableDescriptor;

typedef struct CGPUXMergedBindTableDescriptor {
    CGPURootSignatureId root_signature;
} CGPUXMergedBindTableDescriptor;
