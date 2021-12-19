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

RUNTIME_API const CGpuProcTable* CGPU_VulkanProcTable();
RUNTIME_API const CGpuSurfacesProcTable* CGPU_VulkanSurfacesProcTable();

// Instance APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance_vulkan(CGpuInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_vulkan(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_vulkan(CGpuInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters_vulkan(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);
RUNTIME_API const CGpuAdapterDetail* cgpu_query_adapter_detail_vulkan(const CGpuAdapterId adapter);
RUNTIME_API uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type);

// Device APIs
RUNTIME_API CGpuDeviceId cgpu_create_device_vulkan(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device_vulkan(CGpuDeviceId device);

// API Object APIs
RUNTIME_API CGpuFenceId cgpu_create_fence_vulkan(CGpuDeviceId device);
RUNTIME_API void cgpu_free_fence_vulkan(CGpuFenceId fence);
RUNTIME_API CGpuRootSignatureId cgpu_create_root_signature_vulkan(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc);
RUNTIME_API void cgpu_free_root_signature_vulkan(CGpuRootSignatureId signature);
RUNTIME_API CGpuDescriptorSetId cgpu_create_descriptor_set_vulkan(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc);
RUNTIME_API void cgpu_update_descriptor_set_vulkan(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count);
RUNTIME_API void cgpu_free_descriptor_set_vulkan(CGpuDescriptorSetId set);
RUNTIME_API CGpuComputePipelineId cgpu_create_compute_pipeline_vulkan(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc);
RUNTIME_API void cgpu_free_compute_pipeline_vulkan(CGpuComputePipelineId pipeline);
RUNTIME_API CGpuRenderPipelineId cgpu_create_render_pipeline_vulkan(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc);
RUNTIME_API void cgpu_free_render_pipeline_vulkan(CGpuRenderPipelineId pipeline);

// Queue APIs
RUNTIME_API CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_submit_queue_vulkan(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc);
RUNTIME_API void cgpu_wait_queue_idle_vulkan(CGpuQueueId queue);
RUNTIME_API void cgpu_free_queue_vulkan(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool_vulkan(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc);
RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer_vulkan(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc);
RUNTIME_API void cgpu_reset_command_pool_vulkan(CGpuCommandPoolId pool);
RUNTIME_API void cgpu_free_command_buffer_vulkan(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_free_command_pool_vulkan(CGpuCommandPoolId pool);

// Shader APIs
RUNTIME_API CGpuShaderLibraryId cgpu_create_shader_library_vulkan(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library_vulkan(CGpuShaderLibraryId shader_module);

// Buffer APIs
RUNTIME_API CGpuBufferId cgpu_create_buffer_vulkan(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc);
RUNTIME_API void cgpu_map_buffer_vulkan(CGpuBufferId buffer, const struct CGpuBufferRange* range);
RUNTIME_API void cgpu_unmap_buffer_vulkan(CGpuBufferId buffer);
RUNTIME_API void cgpu_free_buffer_vulkan(CGpuBufferId buffer);

// Texture/RenderTarget APIs
RUNTIME_API CGpuTextureId cgpu_create_texture_vulkan(CGpuDeviceId device, const struct CGpuTextureDescriptor* desc);
RUNTIME_API void cgpu_free_texture_vulkan(CGpuTextureId texture);
RUNTIME_API CGpuRenderTargetId cgpu_create_render_target_vulkan(CGpuDeviceId device, const struct CGpuRenderTargetDescriptor* desc);
RUNTIME_API void cgpu_free_render_target_vulkan(CGpuRenderTargetId render_target);

// Swapchain APIs
RUNTIME_API CGpuSwapChainId cgpu_create_swapchain_vulkan(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain_vulkan(CGpuSwapChainId swapchain);

// CMDs
RUNTIME_API void cgpu_cmd_begin_vulkan(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_cmd_update_buffer_vulkan(CGpuCommandBufferId cmd, const struct CGpuBufferUpdateDescriptor* desc);
RUNTIME_API void cgpu_cmd_resource_barrier_vulkan(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc);
RUNTIME_API void cgpu_cmd_end_vulkan(CGpuCommandBufferId cmd);

// Compute CMDs
RUNTIME_API CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass_vulkan(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc);
RUNTIME_API void cgpu_compute_encoder_bind_descriptor_set_vulkan(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId descriptor);
RUNTIME_API void cgpu_compute_encoder_bind_pipeline_vulkan(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline);
RUNTIME_API void cgpu_compute_encoder_dispatch_vulkan(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
RUNTIME_API void cgpu_cmd_end_compute_pass_vulkan(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder);

// Render CMDs
RUNTIME_API CGpuRenderPassEncoderId cgpu_cmd_begin_render_pass_vulkan(CGpuCommandBufferId cmd, const struct CGpuRenderPassDescriptor* desc);
RUNTIME_API void cgpu_cmd_end_render_pass_vulkan(CGpuCommandBufferId cmd, CGpuRenderPassEncoderId encoder);

typedef struct CGpuInstance_Vulkan {
    CGpuInstance super;
    VkInstance pVkInstance;
    VkDebugUtilsMessengerEXT pVkDebugUtilsMessenger;
    VkDebugReportCallbackEXT pVkDebugReport;
    struct CGpuAdapter_Vulkan* pVulkanAdapters;
    uint32_t mPhysicalDeviceCount;

    // Layers of Instance
    uint32_t mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char** pLayerNames;
    // Enabled Layers Table
    struct CGpuVkLayersTable* pLayersTable;

    // Extension Properties of Instance
    uint32_t mExtensionsCount;
    const char** pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Extensions Table
    struct CGpuVkExtensionsTable* pExtensionsTable;

    // Some Extension Queries
    uint32_t device_group_creation : 1;
    uint32_t debug_utils : 1;
    uint32_t debug_report : 1;
} CGpuInstance_Vulkan;

typedef struct CGpuAdapter_Vulkan {
    CGpuAdapter super;
    VkPhysicalDevice pPhysicalDevice;
    /// Physical Device Props & Features
    VkPhysicalDeviceProperties2 mPhysicalDeviceProps;
    VkPhysicalDeviceFeatures2 mPhysicalDeviceFeatures;
    VkPhysicalDeviceSubgroupProperties mSubgroupProperties;
    /// Queue Families
    uint32_t mQueueFamiliesCount;
    int64_t mQueueFamilyIndices[ECGpuQueueType_Count];
    struct VkQueueFamilyProperties* pQueueFamilyProperties;

    // Layers of Physical Device
    uint32_t mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char** pLayerNames;
    // Enabled Layers Table
    struct CGpuVkLayersTable* pLayersTable;

    // Extension Properties of Physical Device
    uint32_t mExtensionsCount;
    const char** pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Device Extensions Table
    struct CGpuVkExtensionsTable* pExtensionsTable;

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
    uint32_t nv_diagnostic_checkpoints : 1;
    uint32_t nv_diagnostic_config : 1;

    CGpuAdapterDetail adapter_detail;
} CGpuAdapter_Vulkan;

typedef struct CGpuDevice_Vulkan {
    const CGpuDevice super;
    VkDevice pVkDevice;
    VkPipelineCache pPipelineCache;
    struct VkUtil_DescriptorPool* pDescriptorPool;
    struct VmaAllocator_T* pVmaAllocator;
    struct VolkDeviceTable mVkDeviceTable;
} CGpuDevice_Vulkan;

typedef struct CGpuFence_Vulkan {
    CGpuFence super;
    VkFence pVkFence;
    uint32_t mSubmitted : 1;
} CGpuFence_Vulkan;

typedef struct CGpuQueue_Vulkan {
    const CGpuQueue super;
    VkQueue pVkQueue;
    float mTimestampPeriod;
    uint32_t mVkQueueFamilyIndex : 5;
} CGpuQueue_Vulkan;

typedef struct CGpuCommandPool_Vulkan {
    CGpuCommandPool super;
    VkCommandPool pVkCmdPool;
} CGpuCommandPool_Vulkan;

typedef struct CGpuCommandBuffer_Vulkan {
    CGpuCommandBuffer super;
    VkCommandBuffer pVkCmdBuf;
    VkPipelineLayout pBoundPipelineLayout;
    VkRenderPass pRenderPass;
    uint32_t mNodeIndex : 4;
    uint32_t mType : 3;
} CGpuCommandBuffer_Vulkan;

typedef struct CGpuBuffer_Vulkan {
    CGpuBuffer super;
    VkBuffer pVkBuffer;
    VkBufferView pVkStorageTexelView;
    VkBufferView pVkUniformTexelView;
    struct VmaAllocation_T* pVkAllocation;
    uint64_t mOffset;
} CGpuBuffer_Vulkan;

typedef struct CGpuTexture_Vulkan {
    CGpuTexture super;
    /// Opaque handle used by shaders for doing read/write operations on the texture
    VkImageView pVkSRVDescriptor;
    /// Opaque handle used by shaders for doing read/write operations on the texture
    VkImageView* pVkUAVDescriptors;
    /// Opaque handle used by shaders for doing read/write operations on the texture
    VkImageView pVkSRVStencilDescriptor;
    /// Native handle of the underlying resource
    VkImage pVkImage;
    union
    {
        /// Contains resource allocation info such as parent heap, offset in heap
        struct VmaAllocation_T* pVkAllocation;
        VkDeviceMemory pVkDeviceMemory;
    };
} CGpuTexture_Vulkan;

typedef struct CGpuRenderTarget_Vulkan {
    CGpuRenderTarget super;
    VkImageView pVkDescriptor;
    VkImageView* pVkSliceDescriptors;
    uint32_t mId;
} CGpuRenderTarget_Vulkan;

typedef struct CGpuShaderLibrary_Vulkan {
    CGpuShaderLibrary super;
    VkShaderModule mShaderModule;
    struct SpvReflectShaderModule* pReflect;
} CGpuShaderLibrary_Vulkan;

typedef struct CGpuSwapChain_Vulkan {
    CGpuSwapChain super;
    VkSurfaceKHR pVkSurface;
    VkSwapchainKHR pVkSwapChain;
} CGpuSwapChain_Vulkan;

typedef struct ParameterSet_Vulkan {
    VkDescriptorSetLayout layout;
    VkDescriptorUpdateTemplate update_template;
    CGpuShaderResource* resources; // This should be stored here because shader
    uint32_t resources_count;
} ParameterSet_Vulkan;

typedef struct CGpuRootSignature_Vulkan {
    CGpuRootSignature super;
    uint32_t set_count;
    VkPipelineLayout pipeline_layout;
    ParameterSet_Vulkan* parameter_sets;
} CGpuRootSignature_Vulkan;

typedef union VkDescriptorUpdateData
{
    VkDescriptorImageInfo mImageInfo;
    VkDescriptorBufferInfo mBufferInfo;
    VkBufferView mBuferView;
} VkDescriptorUpdateData;

typedef struct CGpuDescriptorSet_Vulkan {
    CGpuDescriptorSet super;
    VkDescriptorSet pVkDescriptorSet;
    union VkDescriptorUpdateData* pUpdateData;
} CGpuDescriptorSet_Vulkan;

typedef struct CGpuComputePipeline_Vulkan {
    CGpuComputePipeline super;
    VkPipeline pVkPipeline;
} CGpuComputePipeline_Vulkan;

typedef struct CGpuRenderPipeline_Vulkan {
    CGpuRenderPipeline super;
    VkPipeline pVkPipeline;
    VkRenderPass pRenderPass;
} CGpuRenderPipeline_Vulkan;

static const VkPipelineBindPoint gPipelineBindPoint[PT_COUNT] = {
    VK_PIPELINE_BIND_POINT_MAX_ENUM,
    VK_PIPELINE_BIND_POINT_COMPUTE,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
#ifdef ENABLE_RAYTRACING
    VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
#endif
};

#ifdef __cplusplus
} // end extern "C"
#endif

#ifdef __cplusplus
extern "C" {
#endif

FORCEINLINE static VkFormat VkUtil_FormatTranslateToVk(const ECGpuFormat format);

#include "cgpu_vulkan.inl"
#ifdef __cplusplus
} // end extern "C"
#endif