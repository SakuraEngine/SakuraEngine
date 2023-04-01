#pragma once
#include "cgpu/api.h"
#if defined(_WIN32) || defined(_WIN64)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "cgpu/backend/vulkan/volk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_VkAllocationCallbacks CGPU_NULLPTR
#define MAX_PLANE_COUNT 3

#ifndef VK_USE_VOLK_DEVICE_TABLE
    #define VK_USE_VOLK_DEVICE_TABLE
#endif

#if VK_HEADER_VERSION >= 135
#define VK_CAN_USE_NSIGHT_AFTERMATH
#endif

RUNTIME_API const CGPUProcTable* CGPU_VulkanProcTable();
RUNTIME_API const CGPUSurfacesProcTable* CGPU_VulkanSurfacesProcTable();

// Instance APIs
RUNTIME_API CGPUInstanceId cgpu_create_instance_vulkan(CGPUInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_vulkan(CGPUInstanceId instance, struct CGPUInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_vulkan(CGPUInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters_vulkan(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num);
RUNTIME_API const CGPUAdapterDetail* cgpu_query_adapter_detail_vulkan(const CGPUAdapterId adapter);
RUNTIME_API uint32_t cgpu_query_queue_count_vulkan(const CGPUAdapterId adapter, const ECGPUQueueType type);

// Device APIs
RUNTIME_API CGPUDeviceId cgpu_create_device_vulkan(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc);
RUNTIME_API void cgpu_query_video_memory_info_vulkan(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
RUNTIME_API void cgpu_query_shared_memory_info_vulkan(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
RUNTIME_API void cgpu_free_device_vulkan(CGPUDeviceId device);

// API Object APIs
RUNTIME_API CGPUFenceId cgpu_create_fence_vulkan(CGPUDeviceId device);
RUNTIME_API void cgpu_wait_fences_vulkan(const CGPUFenceId* fences, uint32_t fence_count);
ECGPUFenceStatus cgpu_query_fence_status_vulkan(CGPUFenceId fence);
RUNTIME_API void cgpu_free_fence_vulkan(CGPUFenceId fence);
RUNTIME_API CGPUSemaphoreId cgpu_create_semaphore_vulkan(CGPUDeviceId device);
RUNTIME_API void cgpu_free_semaphore_vulkan(CGPUSemaphoreId semaphore);
RUNTIME_API CGPURootSignatureId cgpu_create_root_signature_vulkan(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc);
RUNTIME_API void cgpu_free_root_signature_vulkan(CGPURootSignatureId signature);
RUNTIME_API CGPURootSignaturePoolId cgpu_create_root_signature_pool_vulkan(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc);
RUNTIME_API void cgpu_free_root_signature_pool_vulkan(CGPURootSignaturePoolId pool);
RUNTIME_API CGPUDescriptorSetId cgpu_create_descriptor_set_vulkan(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc);
RUNTIME_API void cgpu_update_descriptor_set_vulkan(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count);
RUNTIME_API void cgpu_free_descriptor_set_vulkan(CGPUDescriptorSetId set);
RUNTIME_API CGPUComputePipelineId cgpu_create_compute_pipeline_vulkan(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc);
RUNTIME_API void cgpu_free_compute_pipeline_vulkan(CGPUComputePipelineId pipeline);
RUNTIME_API CGPURenderPipelineId cgpu_create_render_pipeline_vulkan(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc);
RUNTIME_API void cgpu_free_render_pipeline_vulkan(CGPURenderPipelineId pipeline);
RUNTIME_API CGPUQueryPoolId cgpu_create_query_pool_vulkan(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc);
RUNTIME_API void cgpu_free_query_pool_vulkan(CGPUQueryPoolId pool);

// Queue APIs
RUNTIME_API CGPUQueueId cgpu_get_queue_vulkan(CGPUDeviceId device, ECGPUQueueType type, uint32_t index);
RUNTIME_API void cgpu_submit_queue_vulkan(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc);
RUNTIME_API void cgpu_wait_queue_idle_vulkan(CGPUQueueId queue);
RUNTIME_API void cgpu_queue_present_vulkan(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc);
RUNTIME_API float cgpu_queue_get_timestamp_period_ns_vulkan(CGPUQueueId queue);
RUNTIME_API void cgpu_free_queue_vulkan(CGPUQueueId queue);

// Command APIs
RUNTIME_API CGPUCommandPoolId cgpu_create_command_pool_vulkan(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc);
RUNTIME_API CGPUCommandBufferId cgpu_create_command_buffer_vulkan(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc);
RUNTIME_API void cgpu_reset_command_pool_vulkan(CGPUCommandPoolId pool);
RUNTIME_API void cgpu_free_command_buffer_vulkan(CGPUCommandBufferId cmd);
RUNTIME_API void cgpu_free_command_pool_vulkan(CGPUCommandPoolId pool);

// Shader APIs
RUNTIME_API CGPUShaderLibraryId cgpu_create_shader_library_vulkan(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library_vulkan(CGPUShaderLibraryId shader_module);

// Buffer APIs
RUNTIME_API CGPUBufferId cgpu_create_buffer_vulkan(CGPUDeviceId device, const struct CGPUBufferDescriptor* desc);
RUNTIME_API void cgpu_map_buffer_vulkan(CGPUBufferId buffer, const struct CGPUBufferRange* range);
RUNTIME_API void cgpu_unmap_buffer_vulkan(CGPUBufferId buffer);
RUNTIME_API void cgpu_free_buffer_vulkan(CGPUBufferId buffer);

// Sampler APIs
RUNTIME_API CGPUSamplerId cgpu_create_sampler_vulkan(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc);
RUNTIME_API void cgpu_free_sampler_vulkan(CGPUSamplerId sampler);

// Texture/TextureView APIs
RUNTIME_API CGPUTextureId cgpu_create_texture_vulkan(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc);
RUNTIME_API void cgpu_free_texture_vulkan(CGPUTextureId texture);
RUNTIME_API CGPUTextureViewId cgpu_create_texture_view_vulkan(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc);
RUNTIME_API void cgpu_free_texture_view_vulkan(CGPUTextureViewId render_target);
RUNTIME_API bool cgpu_try_bind_aliasing_texture_vulkan(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc);

// Shared Resource APIs
uint64_t cgpu_export_shared_texture_handle_vulkan(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc);
CGPUTextureId cgpu_import_shared_texture_handle_vulkan(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc);

// Swapchain APIs
RUNTIME_API CGPUSwapChainId cgpu_create_swapchain_vulkan(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc);
RUNTIME_API uint32_t cgpu_acquire_next_image_vulkan(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain_vulkan(CGPUSwapChainId swapchain);

// CMDs
RUNTIME_API void cgpu_cmd_begin_vulkan(CGPUCommandBufferId cmd);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_buffer_vulkan(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_texture_vulkan(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc);
RUNTIME_API void cgpu_cmd_transfer_texture_to_texture_vulkan(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc);
RUNTIME_API void cgpu_cmd_resource_barrier_vulkan(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc);
RUNTIME_API void cgpu_cmd_begin_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
RUNTIME_API void cgpu_cmd_end_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
RUNTIME_API void cgpu_cmd_reset_query_pool_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId, uint32_t start_query, uint32_t query_count);
RUNTIME_API void cgpu_cmd_resolve_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count);
RUNTIME_API void cgpu_cmd_end_vulkan(CGPUCommandBufferId cmd);

// Events
RUNTIME_API void cgpu_cmd_begin_event_vulkan(CGPUCommandBufferId cmd, const CGPUEventInfo* event);
RUNTIME_API void cgpu_cmd_set_marker_vulkan(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker);
RUNTIME_API void cgpu_cmd_end_event_vulkan(CGPUCommandBufferId cmd);

// Compute CMDs
RUNTIME_API CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass_vulkan(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc);
RUNTIME_API void cgpu_compute_encoder_bind_descriptor_set_vulkan(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId descriptor);
RUNTIME_API void cgpu_compute_encoder_push_constants_vulkan(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
RUNTIME_API void cgpu_compute_encoder_bind_pipeline_vulkan(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline);
RUNTIME_API void cgpu_compute_encoder_dispatch_vulkan(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
RUNTIME_API void cgpu_cmd_end_compute_pass_vulkan(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder);

// Render CMDs
RUNTIME_API CGPURenderPassEncoderId cgpu_cmd_begin_render_pass_vulkan(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc);
RUNTIME_API void cgpu_render_encoder_set_shading_rate_vulkan(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate);
RUNTIME_API void cgpu_render_encoder_bind_descriptor_set_vulkan(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set);
RUNTIME_API void cgpu_render_encoder_set_viewport_vulkan(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
RUNTIME_API void cgpu_render_encoder_set_scissor_vulkan(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
RUNTIME_API void cgpu_render_encoder_bind_pipeline_vulkan(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline);
RUNTIME_API void cgpu_render_encoder_bind_vertex_buffers_vulkan(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
RUNTIME_API void cgpu_render_encoder_bind_index_buffer_vulkan(CGPURenderPassEncoderId encoder, CGPUBufferId buffer, uint32_t index_stride, uint64_t offset);
RUNTIME_API void cgpu_render_encoder_push_constants_vulkan(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
RUNTIME_API void cgpu_render_encoder_draw_vulkan(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
RUNTIME_API void cgpu_render_encoder_draw_instanced_vulkan(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
RUNTIME_API void cgpu_render_encoder_draw_indexed_vulkan(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
RUNTIME_API void cgpu_render_encoder_draw_indexed_instanced_vulkan(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
RUNTIME_API void cgpu_cmd_end_render_pass_vulkan(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder);

typedef struct CGPUInstance_Vulkan {
    CGPUInstance super;
    VkInstance pVkInstance;
    VkDebugUtilsMessengerEXT pVkDebugUtilsMessenger;
    VkDebugReportCallbackEXT pVkDebugReport;
    struct CGPUAdapter_Vulkan* pVulkanAdapters;
    uint32_t mPhysicalDeviceCount;

    // Layers of Instance
    uint32_t mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char** pLayerNames;
    // Enabled Layers Table
    struct CGPUVkLayersTable* pLayersTable;

    // Extension Properties of Instance
    uint32_t mExtensionsCount;
    const char** pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Extensions Table
    struct CGPUVkExtensionsTable* pExtensionsTable;

    // Some Extension Queries
    uint32_t device_group_creation : 1;
    uint32_t debug_utils : 1;
    uint32_t debug_report : 1;
} CGPUInstance_Vulkan;

typedef struct CGPUAdapter_Vulkan {
    CGPUAdapter super;
    VkPhysicalDevice pPhysicalDevice;
    /// Physical Device Props & Features
    VkPhysicalDeviceProperties2KHR mPhysicalDeviceProps;
#if VK_KHR_fragment_shading_rate
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR mPhysicalDeviceFragmentShadingRateProps;
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR mPhysicalDeviceFragmentShadingRateFeatures;
#endif
#if VK_EXT_extended_dynamic_state
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT mPhysicalDeviceExtendedDynamicStateFeatures;
#endif
#if VK_EXT_extended_dynamic_state2
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT mPhysicalDeviceExtendedDynamicState2Features;
#endif
    VkPhysicalDeviceFeatures2 mPhysicalDeviceFeatures;
    VkPhysicalDeviceSubgroupProperties mSubgroupProperties;
    /// Queue Families
    uint32_t mQueueFamiliesCount;
    int64_t mQueueFamilyIndices[CGPU_QUEUE_TYPE_COUNT];
    struct VkQueueFamilyProperties* pQueueFamilyProperties;

    // Layers of Physical Device
    uint32_t mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char** pLayerNames;
    // Enabled Layers Table
    struct CGPUVkLayersTable* pLayersTable;

    // Extension Properties of Physical Device
    uint32_t mExtensionsCount;
    const char** pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Device Extensions Table
    struct CGPUVkExtensionsTable* pExtensionsTable;

    // Some Extension Queries
    uint32_t debug_marker : 1;
    uint32_t dedicated_allocation : 1;
    uint32_t memory_req2 : 1;
    uint32_t external_memory : 1;
    uint32_t external_memory_win32 : 1;
    uint32_t draw_indirect_count : 1;
    uint32_t amd_draw_indirect_count : 1;
    uint32_t amd_gcn_shader : 1;
    uint32_t descriptor_indexing : 1;
    uint32_t sampler_ycbcr : 1;
#ifdef ENABLE_NSIGHT_AFTERMATH
    uint32_t nv_diagnostic_checkpoints : 1;
    uint32_t nv_diagnostic_config : 1;
#endif
    CGPUAdapterDetail adapter_detail;
} CGPUAdapter_Vulkan;

typedef struct CGPUDevice_Vulkan {
    CGPUDevice super;
    VkDevice pVkDevice;
    VkPipelineCache pPipelineCache;
    struct VkUtil_DescriptorPool* pDescriptorPool;
    struct VmaAllocator_T* pVmaAllocator;
    struct VmaPool_T* pExternalMemoryVmaPools[VK_MAX_MEMORY_TYPES];
    void* pExternalMemoryVmaPoolNexts[VK_MAX_MEMORY_TYPES];
    struct VmaPool_T* pDedicatedAllocationVmaPools[VK_MAX_MEMORY_TYPES];
    struct VolkDeviceTable mVkDeviceTable;
    // Created renderpass table
    struct CGPUVkPassTable* pPassTable;
    uint32_t next_shared_id;
} CGPUDevice_Vulkan;

typedef struct CGPUFence_Vulkan {
    CGPUFence super;
    VkFence pVkFence;
    uint32_t mSubmitted : 1;
} CGPUFence_Vulkan;

typedef struct CGPUSemaphore_Vulkan {
    CGPUSemaphore super;
    VkSemaphore pVkSemaphore;
    uint8_t mSignaled : 1;
} CGPUSemaphore_Vulkan;

typedef struct CGPUQueue_Vulkan {
    const CGPUQueue super;
    VkQueue pVkQueue;
    uint32_t mVkQueueFamilyIndex : 5;
    // Cmd pool for inner usage like resource transition
    CGPUCommandPoolId pInnerCmdPool;
    CGPUCommandBufferId pInnerCmdBuffer;
    CGPUFenceId pInnerFence;
    /// Lock for multi-threaded descriptor allocations
    struct SMutex* pMutex;
} CGPUQueue_Vulkan;

typedef struct CGPUCommandPool_Vulkan {
    CGPUCommandPool super;
    VkCommandPool pVkCmdPool;
} CGPUCommandPool_Vulkan;

typedef struct CGPUQueryPool_Vulkan {
    CGPUQueryPool super;
    VkQueryPool pVkQueryPool;
    VkQueryType mType;
} CGPUQueryPool_Vulkan;

typedef struct CGPUCommandBuffer_Vulkan {
    CGPUCommandBuffer super;
    VkCommandBuffer pVkCmdBuf;
    VkPipelineLayout pBoundPipelineLayout;
    VkRenderPass pRenderPass;
    uint32_t mNodeIndex : 4;
    uint32_t mType : 3;
} CGPUCommandBuffer_Vulkan;

typedef struct CGPUBuffer_Vulkan {
    CGPUBuffer super;
    VkBuffer pVkBuffer;
    VkBufferView pVkStorageTexelView;
    VkBufferView pVkUniformTexelView;
    struct VmaAllocation_T* pVkAllocation;
    uint64_t mOffset;
} CGPUBuffer_Vulkan;

typedef struct CGPUTexture_Vulkan {
    CGPUTexture super;
    VkImage pVkImage;
    union
    {
        /// Contains resource allocation info such as parent heap, offset in heap
        struct VmaAllocation_T* pVkAllocation;
        VkDeviceMemory pVkDeviceMemory;
    };
} CGPUTexture_Vulkan;

typedef struct CGPUTextureView_Vulkan {
    CGPUTextureView super;
    VkImageView pVkRTVDSVDescriptor;
    VkImageView pVkSRVDescriptor;
    VkImageView pVkUAVDescriptor;
} CGPUTextureView_Vulkan;

typedef struct CGPUSampler_Vulkan {
    CGPUSampler super;
    VkSampler pVkSampler;
} CGPUSampler_Vulkan;

typedef struct CGPUShaderLibrary_Vulkan {
    CGPUShaderLibrary super;
    VkShaderModule mShaderModule;
    struct SpvReflectShaderModule* pReflect;
} CGPUShaderLibrary_Vulkan;

typedef struct CGPUSwapChain_Vulkan {
    CGPUSwapChain super;
    VkSurfaceKHR pVkSurface;
    VkSwapchainKHR pVkSwapChain;
} CGPUSwapChain_Vulkan;

typedef struct SetLayout_Vulkan {
    VkDescriptorSetLayout layout;
    VkDescriptorUpdateTemplate pUpdateTemplate;
    uint32_t mUpdateEntriesCount;
    VkDescriptorSet pEmptyDescSet;
} SetLayout_Vulkan;

typedef struct CGPURootSignature_Vulkan {
    CGPURootSignature super;
    VkPipelineLayout pPipelineLayout;
    SetLayout_Vulkan* pSetLayouts;
    uint32_t mSetLayoutCount;
    VkPushConstantRange* pPushConstRanges;
} CGPURootSignature_Vulkan;

typedef union VkDescriptorUpdateData
{
    VkDescriptorImageInfo mImageInfo;
    VkDescriptorBufferInfo mBufferInfo;
    VkBufferView mBuferView;
} VkDescriptorUpdateData;

typedef struct CGPUDescriptorSet_Vulkan {
    CGPUDescriptorSet super;
    VkDescriptorSet pVkDescriptorSet;
    union VkDescriptorUpdateData* pUpdateData;
} CGPUDescriptorSet_Vulkan;

typedef struct CGPUComputePipeline_Vulkan {
    CGPUComputePipeline super;
    VkPipeline pVkPipeline;
} CGPUComputePipeline_Vulkan;

typedef struct CGPURenderPipeline_Vulkan {
    CGPURenderPipeline super;
    VkPipeline pVkPipeline;
} CGPURenderPipeline_Vulkan;

static const VkPipelineBindPoint gPipelineBindPoint[CGPU_PIPELINE_TYPE_COUNT] = {
    VK_PIPELINE_BIND_POINT_MAX_ENUM,
    VK_PIPELINE_BIND_POINT_COMPUTE,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
#ifdef ENABLE_RAYTRACING
    VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
#endif
};

static const VkAttachmentStoreOp gVkAttachmentStoreOpTranslator[CGPU_STORE_ACTION_COUNT] = {
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE
};
static const VkAttachmentLoadOp gVkAttachmentLoadOpTranslator[CGPU_LOAD_ACTION_COUNT] = {
    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    VK_ATTACHMENT_LOAD_OP_LOAD,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
};

static const VkCompareOp gVkComparisonFuncTranslator[CGPU_CMP_COUNT] = {
    VK_COMPARE_OP_NEVER,
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_LESS_OR_EQUAL,
    VK_COMPARE_OP_GREATER,
    VK_COMPARE_OP_NOT_EQUAL,
    VK_COMPARE_OP_GREATER_OR_EQUAL,
    VK_COMPARE_OP_ALWAYS,
};

static const VkStencilOp gVkStencilOpTranslator[CGPU_STENCIL_OP_COUNT] = {
    VK_STENCIL_OP_KEEP,
    VK_STENCIL_OP_ZERO,
    VK_STENCIL_OP_REPLACE,
    VK_STENCIL_OP_INVERT,
    VK_STENCIL_OP_INCREMENT_AND_WRAP,
    VK_STENCIL_OP_DECREMENT_AND_WRAP,
    VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    VK_STENCIL_OP_DECREMENT_AND_CLAMP,
};

#ifdef __cplusplus
} // end extern "C"
#endif

#ifdef __cplusplus
extern "C" {
#endif

FORCEINLINE static VkFormat VkUtil_FormatTranslateToVk(const ECGPUFormat format);

#include "cgpu_vulkan.inl"
#ifdef __cplusplus
} // end extern "C"
#endif