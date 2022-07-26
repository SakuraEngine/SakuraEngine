#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "cgpu/api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ISkrRenderer ISkrRenderer;

typedef struct skr_vertex_buffer_view_t {
    CGPUBufferId buffer;
    // in bytes
    uint32_t offset;
    // in bytes
    uint32_t stride;
} skr_vertex_buffer_view_t;

typedef struct skr_index_buffer_view_t {
    CGPUBufferId buffer;
    // in bytes
    uint32_t offset;
    // in bytes
    uint32_t stride;
    uint32_t index_count;
    uint32_t first_index;
} skr_index_buffer_view_t;

typedef struct skr_primitive_draw_t {
    CGPURenderPipelineId pipeline;
    const char* push_const_name;
    const uint8_t* push_const;
    const skr_vertex_buffer_view_t* vertex_buffers;
    uint32_t vertex_buffer_count;
    skr_index_buffer_view_t index_buffer;
} skr_primitive_draw_t;

typedef struct skr_primitive_draw_list_view_t {
    skr_primitive_draw_t* drawcalls;
    uint32_t count;
} skr_primitive_draw_list_view_t;

#ifdef __cplusplus
}
#endif