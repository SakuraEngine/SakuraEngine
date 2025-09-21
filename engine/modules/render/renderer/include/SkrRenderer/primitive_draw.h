#pragma once
#include "fwd_types.h" // IWYU pragma: export
#include "SkrGraphics/api.h"
#include "SkrGraphics/cgpux.h"

#ifdef __cplusplus
#include "SkrContainersDef/span.hpp"
extern "C" {
#endif

typedef struct skr_vertex_buffer_view_t
{
    CGPUBufferId buffer;
    // in bytes
    uint32_t offset;
    // in bytes
    uint32_t stride;
    uint32_t vertex_count;
    uint32_t primitive_index;
    uint32_t index_in_prim;
} skr_vertex_buffer_view_t;

typedef struct skr_index_buffer_view_t
{
    CGPUBufferId buffer;
    // in bytes
    uint32_t offset;
    // in bytes
    uint32_t stride;
    uint32_t index_count;
    uint32_t first_index;
    uint32_t primitive_index;
} skr_index_buffer_view_t;

typedef struct skr_primitive_draw_t
{
    CGPURenderPipelineId pipeline;
    CGPUXBindTableId bind_table;
    const char8_t* push_const_name;
    const uint8_t* push_const;
    const skr_vertex_buffer_view_t* vertex_buffers;
    uint32_t vertex_buffer_count;
    skr_index_buffer_view_t index_buffer;
    bool deprecated;
} skr_primitive_draw_t;

#ifdef __cplusplus
} // extern "C"

namespace skr
{
using VertexBufferView = skr_vertex_buffer_view_t;
using IndexBufferView = skr_index_buffer_view_t;

struct PrimitiveCommand
{
    skr::Span<const skr_vertex_buffer_view_t> vbvs;
    const skr_index_buffer_view_t* ibv;
    uint32_t primitive_index;
    uint32_t material_index;
};
} // namespace skr
#endif