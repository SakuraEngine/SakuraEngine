#pragma once
#include "cgpu/api.h"

#if defined(_WIN32) || defined(_WIN64)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "cgpu/backend/vulkan/volk.h"

RUNTIME_EXTERN_C CGPU_API const VkAllocationCallbacks GCGPUVkAllocationCallbacks;

#ifdef __cplusplus
extern "C" {
#endif

// #define GLOBAL_VkAllocationCallbacks CGPU_NULLPTR
#define GLOBAL_VkAllocationCallbacks (&GCGPUVkAllocationCallbacks)

#define MAX_PLANE_COUNT 3

#ifndef VK_USE_VOLK_DEVICE_TABLE
    #define VK_USE_VOLK_DEVICE_TABLE
#endif

#if VK_HEADER_VERSION >= 135
#define VK_CAN_USE_NSIGHT_AFTERMATH
#endif

CGPU_API const CGPUProcTable* CGPU_VulkanProcTable();
CGPU_API const CGPUSurfacesProcTable* CGPU_VulkanSurfacesProcTable();

// Instance APIs
CGPU_API CGPUInstanceId cgpu_create_instance_vulkan(CGPUInstanceDescriptor const* descriptor);
CGPU_API void cgpu_query_instance_features_vulkan(CGPUInstanceId instance, struct CGPUInstanceFeatures* features);
CGPU_API void cgpu_free_instance_vulkan(CGPUInstanceId instance);

// Adapter APIs
CGPU_API void cgpu_enum_adapters_vulkan(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num);
CGPU_API const CGPUAdapterDetail* cgpu_query_adapter_detail_vulkan(const CGPUAdapterId adapter);
CGPU_API uint32_t cgpu_query_queue_count_vulkan(const CGPUAdapterId adapter, const ECGPUQueueType type);

// Device APIs
CGPU_API CGPUDeviceId cgpu_create_device_vulkan(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc);
CGPU_API void cgpu_query_video_memory_info_vulkan(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
CGPU_API void cgpu_query_shared_memory_info_vulkan(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
CGPU_API void cgpu_free_device_vulkan(CGPUDeviceId device);

// API Object APIs
CGPU_API CGPUFenceId cgpu_create_fence_vulkan(CGPUDeviceId device);
CGPU_API void cgpu_wait_fences_vulkan(const CGPUFenceId* fences, uint32_t fence_count);
ECGPUFenceStatus cgpu_query_fence_status_vulkan(CGPUFenceId fence);
CGPU_API void cgpu_free_fence_vulkan(CGPUFenceId fence);
CGPU_API CGPUSemaphoreId cgpu_create_semaphore_vulkan(CGPUDeviceId device);
CGPU_API void cgpu_free_semaphore_vulkan(CGPUSemaphoreId semaphore);
CGPU_API CGPURootSignatureId cgpu_create_root_signature_vulkan(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc);
CGPU_API void cgpu_free_root_signature_vulkan(CGPURootSignatureId signature);
CGPU_API CGPURootSignaturePoolId cgpu_create_root_signature_pool_vulkan(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc);
CGPU_API void cgpu_free_root_signature_pool_vulkan(CGPURootSignaturePoolId pool);
CGPU_API CGPUDescriptorSetId cgpu_create_descriptor_set_vulkan(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc);
CGPU_API void cgpu_update_descriptor_set_vulkan(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count);
CGPU_API void cgpu_free_descriptor_set_vulkan(CGPUDescriptorSetId set);
CGPU_API CGPUComputePipelineId cgpu_create_compute_pipeline_vulkan(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc);
CGPU_API void cgpu_free_compute_pipeline_vulkan(CGPUComputePipelineId pipeline);
CGPU_API CGPURenderPipelineId cgpu_create_render_pipeline_vulkan(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc);
CGPU_API void cgpu_free_render_pipeline_vulkan(CGPURenderPipelineId pipeline);
CGPU_API CGPUQueryPoolId cgpu_create_query_pool_vulkan(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc);
CGPU_API void cgpu_free_query_pool_vulkan(CGPUQueryPoolId pool);

// Queue APIs
CGPU_API CGPUQueueId cgpu_get_queue_vulkan(CGPUDeviceId device, ECGPUQueueType type, uint32_t index);
CGPU_API void cgpu_submit_queue_vulkan(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc);
CGPU_API void cgpu_wait_queue_idle_vulkan(CGPUQueueId queue);
CGPU_API void cgpu_queue_present_vulkan(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc);
CGPU_API float cgpu_queue_get_timestamp_period_ns_vulkan(CGPUQueueId queue);
CGPU_API void cgpu_queue_map_tiled_texture_vulkan(CGPUQueueId queue, const struct CGPUTiledTextureRegions* regions);
CGPU_API void cgpu_queue_unmap_tiled_texture_vulkan(CGPUQueueId queue, const struct CGPUTiledTextureRegions* regions);
CGPU_API void cgpu_queue_map_packed_mips_vulkan(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions);
CGPU_API void cgpu_queue_unmap_packed_mips_vulkan(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions);
CGPU_API void cgpu_free_queue_vulkan(CGPUQueueId queue);

// Command APIs
CGPU_API CGPUCommandPoolId cgpu_create_command_pool_vulkan(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc);
CGPU_API CGPUCommandBufferId cgpu_create_command_buffer_vulkan(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc);
CGPU_API void cgpu_reset_command_pool_vulkan(CGPUCommandPoolId pool);
CGPU_API void cgpu_free_command_buffer_vulkan(CGPUCommandBufferId cmd);
CGPU_API void cgpu_free_command_pool_vulkan(CGPUCommandPoolId pool);

// Shader APIs
CGPU_API CGPUShaderLibraryId cgpu_create_shader_library_vulkan(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc);
CGPU_API void cgpu_free_shader_library_vulkan(CGPUShaderLibraryId shader_module);

// Buffer APIs
CGPU_API CGPUBufferId cgpu_create_buffer_vulkan(CGPUDeviceId device, const struct CGPUBufferDescriptor* desc);
CGPU_API void cgpu_map_buffer_vulkan(CGPUBufferId buffer, const struct CGPUBufferRange* range);
CGPU_API void cgpu_unmap_buffer_vulkan(CGPUBufferId buffer);
CGPU_API void cgpu_free_buffer_vulkan(CGPUBufferId buffer);

// Sampler APIs
CGPU_API CGPUSamplerId cgpu_create_sampler_vulkan(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc);
CGPU_API void cgpu_free_sampler_vulkan(CGPUSamplerId sampler);

// Texture/TextureView APIs
CGPU_API CGPUTextureId cgpu_create_texture_vulkan(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc);
CGPU_API void cgpu_free_texture_vulkan(CGPUTextureId texture);
CGPU_API CGPUTextureViewId cgpu_create_texture_view_vulkan(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc);
CGPU_API void cgpu_free_texture_view_vulkan(CGPUTextureViewId render_target);
CGPU_API bool cgpu_try_bind_aliasing_texture_vulkan(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc);

// Shared Resource APIs
uint64_t cgpu_export_shared_texture_handle_vulkan(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc);
CGPUTextureId cgpu_import_shared_texture_handle_vulkan(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc);

// Swapchain APIs
CGPU_API CGPUSwapChainId cgpu_create_swapchain_vulkan(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc);
CGPU_API uint32_t cgpu_acquire_next_image_vulkan(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc);
CGPU_API void cgpu_free_swapchain_vulkan(CGPUSwapChainId swapchain);

// CMDs
CGPU_API void cgpu_cmd_begin_vulkan(CGPUCommandBufferId cmd);
CGPU_API void cgpu_cmd_transfer_buffer_to_buffer_vulkan(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc);
CGPU_API void cgpu_cmd_transfer_buffer_to_texture_vulkan(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc);
CGPU_API void cgpu_cmd_transfer_buffer_to_tiles_vulkan(CGPUCommandBufferId cmd, const struct CGPUBufferToTilesTransfer* desc);
CGPU_API void cgpu_cmd_transfer_texture_to_texture_vulkan(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc);
CGPU_API void cgpu_cmd_resource_barrier_vulkan(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc);
CGPU_API void cgpu_cmd_begin_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
CGPU_API void cgpu_cmd_end_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
CGPU_API void cgpu_cmd_reset_query_pool_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId, uint32_t start_query, uint32_t query_count);
CGPU_API void cgpu_cmd_resolve_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count);
CGPU_API void cgpu_cmd_end_vulkan(CGPUCommandBufferId cmd);

// Events
CGPU_API void cgpu_cmd_begin_event_vulkan(CGPUCommandBufferId cmd, const CGPUEventInfo* event);
CGPU_API void cgpu_cmd_set_marker_vulkan(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker);
CGPU_API void cgpu_cmd_end_event_vulkan(CGPUCommandBufferId cmd);

// Compute CMDs
CGPU_API CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass_vulkan(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc);
CGPU_API void cgpu_compute_encoder_bind_descriptor_set_vulkan(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId descriptor);
CGPU_API void cgpu_compute_encoder_push_constants_vulkan(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
CGPU_API void cgpu_compute_encoder_bind_pipeline_vulkan(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline);
CGPU_API void cgpu_compute_encoder_dispatch_vulkan(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
CGPU_API void cgpu_cmd_end_compute_pass_vulkan(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder);

// Render CMDs
CGPU_API CGPURenderPassEncoderId cgpu_cmd_begin_render_pass_vulkan(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc);
CGPU_API void cgpu_render_encoder_set_shading_rate_vulkan(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate);
CGPU_API void cgpu_render_encoder_bind_descriptor_set_vulkan(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set);
CGPU_API void cgpu_render_encoder_set_viewport_vulkan(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
CGPU_API void cgpu_render_encoder_set_scissor_vulkan(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
CGPU_API void cgpu_render_encoder_bind_pipeline_vulkan(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline);
CGPU_API void cgpu_render_encoder_bind_vertex_buffers_vulkan(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
CGPU_API void cgpu_render_encoder_bind_index_buffer_vulkan(CGPURenderPassEncoderId encoder, CGPUBufferId buffer, uint32_t index_stride, uint64_t offset);
CGPU_API void cgpu_render_encoder_push_constants_vulkan(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
CGPU_API void cgpu_render_encoder_draw_vulkan(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
CGPU_API void cgpu_render_encoder_draw_instanced_vulkan(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
CGPU_API void cgpu_render_encoder_draw_indexed_vulkan(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
CGPU_API void cgpu_render_encoder_draw_indexed_instanced_vulkan(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
CGPU_API void cgpu_cmd_end_render_pass_vulkan(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder);

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
#if VK_KHR_depth_stencil_resolve
    VkPhysicalDeviceDepthStencilResolvePropertiesKHR mPhysicalDeviceDepthStencilResolveProps;
#endif
#if VK_KHR_dynamic_rendering
    VkPhysicalDeviceDynamicRenderingFeaturesKHR mPhysicalDeviceDynamicRenderingFeatures;
#endif
#if VK_EXT_extended_dynamic_state
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT mPhysicalDeviceExtendedDynamicStateFeatures;
#endif
#if VK_EXT_extended_dynamic_state2
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT mPhysicalDeviceExtendedDynamicState2Features;
#endif
#if VK_EXT_extended_dynamic_state3 // NVIDIA: driver version >= 531.54
    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT mPhysicalDeviceExtendedDynamicState3Properties;
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT mPhysicalDeviceExtendedDynamicState3Features;
#endif
#if VK_EXT_shader_object // NVIDIA: driver version >= 531.54
    VkPhysicalDeviceShaderObjectFeaturesEXT mPhysicalDeviceShaderObjectFeatures;
    VkPhysicalDeviceShaderObjectPropertiesEXT mPhysicalDeviceShaderObjectProperties;
#endif
#if VK_KHR_buffer_device_address
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR mPhysicalDeviceBufferDeviceAddressFeatures;
#endif
#if VK_EXT_descriptor_buffer
    VkPhysicalDeviceDescriptorBufferFeaturesEXT mPhysicalDeviceDescriptorBufferFeatures;
    VkPhysicalDeviceDescriptorBufferPropertiesEXT mPhysicalDeviceDescriptorBufferProperties;
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
    uint32_t buffer_device_address : 1;
    uint32_t descriptor_buffer : 1;
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
    // struct VmaPool_T* pDedicatedAllocationVmaPools[VK_MAX_MEMORY_TYPES];
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

typedef struct CGPUTileMapping_Vulkan
{
    struct VmaAllocation_T* pVkAllocation;
    volatile int32_t status;
} CGPUTileMapping_Vulkan;

typedef struct CGPUTileTextureSubresourceMapping_Vulkan {
    const uint32_t X;
    const uint32_t Y;
    const uint32_t Z;
    const uint32_t mVkMemoryTypeBits;
    CGPUTileMapping_Vulkan* mappings;
} CGPUTileTextureSubresourceMapping_Vulkan;

typedef struct CGPUTileTexturePackedMipMapping_Vulkan {
    struct VmaAllocation_T* pVkAllocation;
    volatile int32_t status;
    uint64_t mVkSparseTailStride;
    uint64_t mVkSparseTailOffset;
    uint64_t mVkSparseTailSize;
} CGPUTileTexturePackedMipMapping_Vulkan;

typedef struct CGPUTexture_Vulkan {
    CGPUTexture super;
    VkImage pVkImage;
    union
    {
        /// Contains resource allocation info such as parent heap, offset in heap
        struct VmaAllocation_T* pVkAllocation;
        VkDeviceMemory pVkDeviceMemory;
        struct {
            CGPUTileTextureSubresourceMapping_Vulkan* pVkTileMappings;
            CGPUTileTexturePackedMipMapping_Vulkan* pVkPackedMappings;
            uint32_t mPackedMappingsCount;
            bool mSingleTail;
        };
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
    VkDescriptorSetLayout* pVkSetLayouts;
    uint32_t mSetLayoutCount;
    VkPushConstantRange* pPushConstRanges;
} CGPURootSignature_Vulkan;

typedef struct CGPUCompiledShader_Vulkan {
    CGPUCompiledShader super;
    VkShaderEXT pVkShader;
} CGPUCompiledShader_Vulkan;

typedef struct CGPULinkedShader_Vulkan {
    CGPULinkedShader super;
    VkShaderEXT pVkShaders[CGPU_SHADER_STAGE_COUNT];
    ECGPUShaderStage pStages[CGPU_SHADER_STAGE_COUNT];
} CGPULinkedShader_Vulkan;

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