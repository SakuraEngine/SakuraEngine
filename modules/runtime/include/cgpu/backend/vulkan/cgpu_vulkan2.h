#pragma once
#include "cgpu_vulkan.h"

// Compiled/Linked ISA APIs
RUNTIME_API CGPULinkedShaderId cgpu_compile_and_link_shaders_vulkan(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* descs, uint32_t count);
RUNTIME_API void cgpu_compile_shaders_vulkan(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* descs, uint32_t count, CGPUCompiledShaderId* out_isas);
RUNTIME_API void cgpu_free_compiled_shader_vulkan(CGPUCompiledShaderId shader);
RUNTIME_API void cgpu_free_linked_shader_vulkan(CGPULinkedShaderId shader);

// StateStream APIs
RUNTIME_API CGPUStateStreamId cgpu_create_state_stream_vulkan(CGPUDeviceId device, const struct CGPUStateStreamDescriptor* desc);
RUNTIME_API void cgpu_render_encoder_bind_state_stream_vulkan(CGPURenderPassEncoderId encoder, CGPUStateStreamId stream);
RUNTIME_API void cgpu_compute_encoder_bind_state_stream_vulkan(CGPUComputePassEncoderId encoder, CGPUStateStreamId stream);
RUNTIME_API void cgpu_free_state_stream_vulkan(CGPUStateStreamId stream);

// raster state encoder APIs
RUNTIME_API CGPURasterStateEncoderId cgpu_open_raster_state_encoder_vulkan(CGPUStateStreamId stream, CGPURenderPassEncoderId encoder);
RUNTIME_API void cgpu_close_raster_state_encoder_vulkan(CGPURasterStateEncoderId encoder);
// dynamic_state
RUNTIME_API void cgpu_raster_state_encoder_set_viewport_vulkan(CGPURasterStateEncoderId, float x, float y, float width, float height, float min_depth, float max_depth);
RUNTIME_API void cgpu_raster_state_encoder_set_scissor_vulkan(CGPURasterStateEncoderId, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
RUNTIME_API void cgpu_raster_state_encoder_set_cull_mode_vulkan(CGPURasterStateEncoderId, ECGPUCullMode cull_mode);
RUNTIME_API void cgpu_raster_state_encoder_set_front_face_vulkan(CGPURasterStateEncoderId, ECGPUFrontFace front_face);
RUNTIME_API void cgpu_raster_state_encoder_set_primitive_topology_vulkan(CGPURasterStateEncoderId, ECGPUPrimitiveTopology topology);
RUNTIME_API void cgpu_raster_state_encoder_set_depth_test_enabled_vulkan(CGPURasterStateEncoderId, bool enabled);
RUNTIME_API void cgpu_raster_state_encoder_set_depth_write_enabled_vulkan(CGPURasterStateEncoderId, bool enabled);
RUNTIME_API void cgpu_raster_state_encoder_set_depth_compare_op_vulkan(CGPURasterStateEncoderId, ECGPUCompareMode compare_op);
RUNTIME_API void cgpu_raster_state_encoder_set_stencil_test_enabled_vulkan(CGPURasterStateEncoderId, bool enabled);
RUNTIME_API void cgpu_raster_state_encoder_set_stencil_compare_op_vulkan(CGPURasterStateEncoderId, CGPUStencilFaces faces, ECGPUStencilOp failOp, ECGPUStencilOp passOp, ECGPUStencilOp depthFailOp, ECGPUCompareMode compareOp);
// dynamic_state2
// dynamic_state3
RUNTIME_API void cgpu_raster_state_encoder_set_fill_mode_vulkan(CGPURasterStateEncoderId, ECGPUFillMode fill_mode);
RUNTIME_API void cgpu_raster_state_encoder_set_sample_count_vulkan(CGPURasterStateEncoderId, ECGPUSampleCount sample_count);

// shader state encoder APIs
RUNTIME_API CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_r_vulkan(CGPUStateStreamId stream, CGPURenderPassEncoderId encoder);
RUNTIME_API CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_c_vulkan(CGPUStateStreamId stream, CGPUComputePassEncoderId encoder);
RUNTIME_API void cgpu_shader_state_encoder_bind_shaders_vulkan(CGPUShaderStateEncoderId, uint32_t stage_count, const ECGPUShaderStage* stages, const CGPUCompiledShaderId* shaders);
RUNTIME_API void cgpu_shader_state_encoder_bind_linked_shader_vulkan(CGPUShaderStateEncoderId, CGPULinkedShaderId linked);
RUNTIME_API void cgpu_close_shader_state_encoder_vulkan(CGPUShaderStateEncoderId encoder);

// user state encoder APIs
RUNTIME_API CGPUUserStateEncoderId cgpu_open_user_state_encoder_vulkan(CGPUStateStreamId stream, CGPURenderPassEncoderId encoder);
RUNTIME_API void cgpu_close_user_state_encoder_vulkan(CGPUUserStateEncoderId encoder);

// EXPERIMENTAL binder APIs
RUNTIME_API CGPUBinderId cgpu_create_binder_vulkan(CGPURootSignatureId root_signature);
RUNTIME_API void cgpu_binder_bind_vertex_layout_vulkan(CGPUBinderId, const struct CGPUVertexLayout* layout);
RUNTIME_API void cgpu_binder_bind_vertex_buffer_vulkan(CGPUBinderId, uint32_t first_binding, uint32_t binding_count, const CGPUBufferId* buffers, const uint64_t* offsets, const uint64_t* sizes, const uint64_t* strides);
RUNTIME_API void cgpu_free_binder_vulkan(CGPUBinderId binder);

typedef struct CGPUStateStream_Vulkan {
    CGPUStateStream super;
    
} CGPUStateStream_Vulkan;