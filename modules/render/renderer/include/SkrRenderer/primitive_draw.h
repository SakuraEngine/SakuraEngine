#pragma once
#include "fwd_types.h"

#include "SkrGraphics/api.h"
#include "SkrGraphics/cgpux.h"

#ifdef __cplusplus
#include "SkrContainers/span.hpp"
extern "C" {
#endif

typedef struct SRenderer SRenderer;
typedef struct SRenderer* SRendererId;

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
    CGPUXBindTableId bind_table;
    const char8_t* push_const_name;
    const uint8_t* push_const;
    const skr_vertex_buffer_view_t* vertex_buffers;
    uint32_t vertex_buffer_count;
    skr_index_buffer_view_t index_buffer;
    bool desperated;
} skr_primitive_draw_t;

typedef struct skr_primitive_draw_list_view_t {
    skr_primitive_draw_t* drawcalls;
    uint32_t count;
    void* user_data;
} skr_primitive_draw_list_view_t;

typedef struct skr_primitive_draw_packet_t {
    skr_primitive_draw_list_view_t* lists;
    uint32_t count;
    void* user_data;
} skr_primitive_draw_packet_t;

#ifdef __cplusplus
} // extern "C"

namespace skr {
namespace renderer {
using VertexBufferView = skr_vertex_buffer_view_t;
using IndexBufferView = skr_index_buffer_view_t;

struct PrimitiveCommand {
    skr::span<const skr_vertex_buffer_view_t> vbvs;
    const skr_index_buffer_view_t* ibv;
    uint32_t primitive_index;
    uint32_t material_index;
};

} // namespace renderer
} // namespace skr
#endif