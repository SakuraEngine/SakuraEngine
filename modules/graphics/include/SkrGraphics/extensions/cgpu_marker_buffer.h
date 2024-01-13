#pragma once
#include "SkrGraphics/api.h"

typedef struct CGPUMarkerBuffer CGPUMarkerBuffer;
typedef struct CGPUMarkerBuffer* CGPUMarkerBufferId;

typedef struct CGPUMarkerBufferDescriptor {
    uint32_t marker_count;
} CGPUMarkerBufferDescriptor;

typedef struct CGPUMarkerBuffer {
    CGPUBufferId cgpu_buffer;
    uint32_t marker_count;
    uint32_t used_count;
} CGPUMarkerBuffer;

CGPU_EXTERN_C CGPU_API
bool cgpu_device_support_marker_buffer(CGPUDeviceId device);

CGPU_EXTERN_C CGPU_API
CGPUMarkerBufferId cgpu_create_marker_buffer(CGPUDeviceId device, CGPUMarkerBufferDescriptor const* descriptor);

CGPU_EXTERN_C CGPU_API
void cgpu_marker_buffer_write(CGPUCommandBufferId cmd, CGPUMarkerBufferId buffer, uint32_t index, uint32_t value);

CGPU_EXTERN_C CGPU_API
void cgpu_free_marker_buffer(CGPUMarkerBufferId buffer);