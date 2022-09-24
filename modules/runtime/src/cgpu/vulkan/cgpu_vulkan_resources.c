#include "float.h"
#include "math/scalarmath.h"
#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "../common/common_utils.h"
#include "vulkan_utils.h"
#ifdef CGPU_THREAD_SAFETY
    #include "platform/thread.h"
#endif
#include <string.h>

FORCEINLINE static VkBufferCreateInfo VkUtil_CreateBufferCreateInfo(CGPUAdapter_Vulkan* A, const struct CGPUBufferDescriptor* desc)
{
    uint64_t allocationSize = desc->size;
    // Align the buffer size to multiples of the dynamic uniform buffer minimum size
    if (desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
    {
        uint64_t minAlignment = A->adapter_detail.uniform_buffer_alignment;
        allocationSize = smath_round_up(allocationSize, minAlignment);
    }
    VkBufferCreateInfo add_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = allocationSize,
        // Queues Props
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL
    };
    add_info.usage = VkUtil_DescriptorTypesToBufferUsage(desc->descriptors, desc->format != CGPU_FORMAT_UNDEFINED);
    // Buffer can be used as dest in a transfer command (Uploading data to a storage buffer, Readback query data)
    if (desc->memory_usage == CGPU_MEM_USAGE_GPU_ONLY || desc->memory_usage == CGPU_MEM_USAGE_GPU_TO_CPU)
        add_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return add_info;
}

FORCEINLINE static VkFormatFeatureFlags VkUtil_ImageUsageToFormatFeatures(VkImageUsageFlags usage)
{
    VkFormatFeatureFlags result = (VkFormatFeatureFlags)0;
    if (VK_IMAGE_USAGE_SAMPLED_BIT == (usage & VK_IMAGE_USAGE_SAMPLED_BIT))
    {
        result |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    }
    if (VK_IMAGE_USAGE_STORAGE_BIT == (usage & VK_IMAGE_USAGE_STORAGE_BIT))
    {
        result |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    }
    if (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT == (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
    {
        result |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    }
    if (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT == (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        result |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    return result;
}

void cgpu_query_video_memory_info_vulkan(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    const VkPhysicalDeviceMemoryProperties* mem_props = CGPU_NULLPTR;
    vmaGetMemoryProperties(D->pVmaAllocator, &mem_props);
    VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
    vmaGetHeapBudgets(D->pVmaAllocator, budgets);
    *total = 0;
    *used_bytes = 0;
    for (uint32_t i = 0; i < mem_props->memoryHeapCount; i++)
    {
        if (mem_props->memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            *total += budgets[i].budget;
            *used_bytes += budgets[i].usage;
        }
    }
}

void cgpu_query_shared_memory_info_vulkan(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    const VkPhysicalDeviceMemoryProperties* mem_props = CGPU_NULLPTR;
    vmaGetMemoryProperties(D->pVmaAllocator, &mem_props);
    VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
    vmaGetHeapBudgets(D->pVmaAllocator, budgets);
    *total = 0;
    *used_bytes = 0;
    for (uint32_t i = 0; i < mem_props->memoryHeapCount; i++)
    {
        if (mem_props->memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
        else
        {
            *total += budgets[i].budget;
            *used_bytes += budgets[i].usage;
        }
    }
}

// Buffer APIs
cgpu_static_assert(sizeof(CGPUBuffer_Vulkan) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line
CGPUBufferId cgpu_create_buffer_vulkan(CGPUDeviceId device, const struct CGPUBufferDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)device->adapter;
    // Create VkBufferCreateInfo
    VkBufferCreateInfo add_info = VkUtil_CreateBufferCreateInfo(A, desc);
    // VMA Alloc
    VmaAllocationCreateInfo vma_mem_reqs = {
        .usage = (VmaMemoryUsage)desc->memory_usage
    };
    if (desc->flags & CGPU_BCF_OWN_MEMORY_BIT)
        vma_mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    if (desc->flags & CGPU_BCF_PERSISTENT_MAP_BIT)
        vma_mem_reqs.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    if ((desc->flags & CGPU_BCF_HOST_VISIBLE && desc->memory_usage & CGPU_MEM_USAGE_GPU_ONLY) ||
        (desc->flags & CGPU_BCF_PERSISTENT_MAP_BIT && desc->memory_usage & CGPU_MEM_USAGE_GPU_ONLY))
        vma_mem_reqs.preferredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    // VMA recommanded upload & readback usage
    if (desc->memory_usage == CGPU_MEM_USAGE_CPU_TO_GPU)
    {
        vma_mem_reqs.usage =
        desc->prefer_on_device ? VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE :
        desc->prefer_on_host   ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST :
                                 VMA_MEMORY_USAGE_AUTO;
        vma_mem_reqs.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }
    if (desc->memory_usage == CGPU_MEM_USAGE_GPU_TO_CPU)
    {
        vma_mem_reqs.usage =
        desc->prefer_on_device ? VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE :
        desc->prefer_on_host   ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST :
                                 VMA_MEMORY_USAGE_AUTO;
        vma_mem_reqs.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    }
    DECLARE_ZERO(VmaAllocationInfo, alloc_info)
    VkBuffer pVkBuffer = VK_NULL_HANDLE;
    VmaAllocation mVmaAllocation = VK_NULL_HANDLE;
    VkResult bufferResult = vmaCreateBuffer(D->pVmaAllocator, &add_info, &vma_mem_reqs, &pVkBuffer, &mVmaAllocation, &alloc_info);
    if (bufferResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    {
        return CGPU_BUFFER_OUT_OF_DEVICE_MEMORY;
    }
    else if (bufferResult == VK_ERROR_OUT_OF_HOST_MEMORY)
    {
        return CGPU_BUFFER_OUT_OF_HOST_MEMORY;
    }
    else if (bufferResult != VK_SUCCESS)
    {
        cgpu_assert(0 && "VMA failed to create buffer!");
        return CGPU_NULLPTR;
    }
    CGPUBuffer_Vulkan* B = cgpu_calloc_aligned(1, sizeof(CGPUBuffer_Vulkan), _Alignof(CGPUBuffer_Vulkan));
    B->super.cpu_mapped_address = alloc_info.pMappedData;
    B->pVkAllocation = mVmaAllocation;
    B->pVkBuffer = pVkBuffer;

    // Setup Descriptors
    if ((desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER) || (desc->descriptors & CGPU_RESOURCE_TYPE_BUFFER) ||
        (desc->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER))
    {
        if ((desc->descriptors & CGPU_RESOURCE_TYPE_BUFFER) || (desc->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER))
        {
            B->mOffset = desc->element_stride * desc->first_element;
        }
    }
    // Setup Uniform Texel View
    if ((add_info.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) || (add_info.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT))
    {
        const VkFormat texel_format = VkUtil_FormatTranslateToVk(desc->format);
        DECLARE_ZERO(VkFormatProperties, formatProps)
        vkGetPhysicalDeviceFormatProperties(A->pPhysicalDevice, texel_format, &formatProps);
        // Now We Use The Same View Info for Uniform & Storage BufferView on Vulkan Backend.
        VkBufferViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .pNext = NULL,
            .buffer = B->pVkBuffer,
            .flags = 0,
            .format = texel_format,
            .offset = desc->first_element * desc->element_stride,
            .range = desc->elemet_count * desc->element_stride
        };
        if (add_info.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
        {
            if (!(formatProps.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT))
            {
                cgpu_warn("Failed to create uniform texel buffer view for format %u", (uint32_t)desc->format);
            }
            else
            {
                CHECK_VKRESULT(vkCreateBufferView(D->pVkDevice, &viewInfo, GLOBAL_VkAllocationCallbacks, &B->pVkUniformTexelView));
            }
        }
        // Setup Storage Texel View
        if (add_info.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
        {
            if (!(formatProps.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT))
            {
                cgpu_warn("Failed to create storage texel buffer view for format %u", (uint32_t)desc->format);
            }
            else
            {
                CHECK_VKRESULT(vkCreateBufferView(D->pVkDevice, &viewInfo, GLOBAL_VkAllocationCallbacks, &B->pVkStorageTexelView));
            }
        }
    }
    // Set Buffer Name
    VkUtil_OptionalSetObjectName(D, (uint64_t)B->pVkBuffer, VK_OBJECT_TYPE_BUFFER, desc->name);
    // Set Buffer Object Props
    B->super.size = desc->size;
    B->super.memory_usage = desc->memory_usage;
    B->super.descriptors = desc->descriptors;
    // Start state
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)desc->owner_queue;
    if (Q && B->pVkBuffer != VK_NULL_HANDLE && B->pVkAllocation != VK_NULL_HANDLE)
    {
#ifdef CGPU_THREAD_SAFETY
        if (Q->pMutex) skr_acquire_mutex(Q->pMutex);
#endif
        cgpu_reset_command_pool(Q->pInnerCmdPool);
        cgpu_cmd_begin(Q->pInnerCmdBuffer);
        CGPUBufferBarrier init_barrier = {
            .buffer = &B->super,
            .src_state = CGPU_RESOURCE_STATE_UNDEFINED,
            .dst_state = desc->start_state
        };
        CGPUResourceBarrierDescriptor init_barrier_d = {
            .buffer_barriers = &init_barrier,
            .buffer_barriers_count = 1
        };
        cgpu_cmd_resource_barrier(Q->pInnerCmdBuffer, &init_barrier_d);
        cgpu_cmd_end(Q->pInnerCmdBuffer);
        CGPUQueueSubmitDescriptor barrier_submit = {
            .cmds = &Q->pInnerCmdBuffer,
            .cmds_count = 1,
            .signal_fence = Q->pInnerFence
        };
        cgpu_submit_queue(&Q->super, &barrier_submit);
        cgpu_wait_fences(&Q->pInnerFence, 1);
#ifdef CGPU_THREAD_SAFETY
        if (Q->pMutex) skr_release_mutex(Q->pMutex);
#endif
    }
    return &B->super;
}

void cgpu_map_buffer_vulkan(CGPUBufferId buffer, const struct CGPUBufferRange* range)
{
    CGPUBuffer_Vulkan* B = (CGPUBuffer_Vulkan*)buffer;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)B->super.device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)buffer->device->adapter;
    if (!A->adapter_detail.support_host_visible_vram)
        cgpu_assert(B->super.memory_usage != CGPU_MEM_USAGE_GPU_ONLY && "Trying to map non-cpu accessible resource");

    VkResult vk_res = vmaMapMemory(D->pVmaAllocator, B->pVkAllocation, &B->super.cpu_mapped_address);
    cgpu_assert(vk_res == VK_SUCCESS);

    if (range && (vk_res == VK_SUCCESS))
    {
        B->super.cpu_mapped_address = ((uint8_t*)B->super.cpu_mapped_address + range->offset);
    }
}

void cgpu_unmap_buffer_vulkan(CGPUBufferId buffer)
{
    CGPUBuffer_Vulkan* B = (CGPUBuffer_Vulkan*)buffer;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)B->super.device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)buffer->device->adapter;
    if (!A->adapter_detail.support_host_visible_vram)
        cgpu_assert(B->super.memory_usage != CGPU_MEM_USAGE_GPU_ONLY && "Trying to unmap non-cpu accessible resource");

    vmaUnmapMemory(D->pVmaAllocator, B->pVkAllocation);
    B->super.cpu_mapped_address = CGPU_NULLPTR;
}

void cgpu_cmd_transfer_buffer_to_buffer_vulkan(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUBuffer_Vulkan* Dst = (CGPUBuffer_Vulkan*)desc->dst;
    CGPUBuffer_Vulkan* Src = (CGPUBuffer_Vulkan*)desc->src;
    VkBufferCopy region = {
        .srcOffset = desc->src_offset,
        .dstOffset = desc->dst_offset,
        .size = desc->size
    };
    D->mVkDeviceTable.vkCmdCopyBuffer(Cmd->pVkCmdBuf, Src->pVkBuffer, Dst->pVkBuffer, 1, &region);
}

void cgpu_cmd_transfer_buffer_to_texture_vulkan(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUTexture_Vulkan* Dst = (CGPUTexture_Vulkan*)desc->dst;
    CGPUBuffer_Vulkan* Src = (CGPUBuffer_Vulkan*)desc->src;
    const bool isSinglePlane = true;
    const ECGPUFormat fmt = desc->dst->format;
    if (isSinglePlane)
    {
        const uint32_t width = cgpu_max(1, desc->dst->width >> desc->dst_subresource.mip_level);
        const uint32_t height = cgpu_max(1, desc->dst->height >> desc->dst_subresource.mip_level);
        const uint32_t depth = cgpu_max(1, desc->dst->depth >> desc->dst_subresource.mip_level);

        VkBufferImageCopy copy = {
            .bufferOffset = desc->src_offset,
            .bufferRowLength = width * FormatUtil_WidthOfBlock(fmt),
            .bufferImageHeight = height * FormatUtil_HeightOfBlock(fmt),
            .imageSubresource.aspectMask = (VkImageAspectFlags)desc->dst->aspect_mask,
            .imageSubresource.mipLevel = desc->dst_subresource.mip_level,
            .imageSubresource.baseArrayLayer = desc->dst_subresource.base_array_layer,
            .imageSubresource.layerCount = desc->dst_subresource.layer_count,
            .imageOffset.x = 0,
            .imageOffset.y = 0,
            .imageOffset.z = 0,
            .imageExtent.width = width,
            .imageExtent.height = height,
            .imageExtent.depth = depth
        };
        D->mVkDeviceTable.vkCmdCopyBufferToImage(Cmd->pVkCmdBuf,
        Src->pVkBuffer, Dst->pVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
        &copy);
    }
}

void cgpu_cmd_transfer_texture_to_texture_vulkan(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUTexture_Vulkan* Dst = (CGPUTexture_Vulkan*)desc->dst;
    CGPUTexture_Vulkan* Src = (CGPUTexture_Vulkan*)desc->src;
    const bool isSinglePlane = true;
    if (isSinglePlane)
    {
        const uint32_t width = cgpu_max(1, desc->dst->width >> desc->dst_subresource.mip_level);
        const uint32_t height = cgpu_max(1, desc->dst->height >> desc->dst_subresource.mip_level);
        const uint32_t depth = cgpu_max(1, desc->dst->depth >> desc->dst_subresource.mip_level);

        VkImageCopy copy_region = {
            .srcSubresource = {
                desc->src_subresource.aspects,
                desc->src_subresource.mip_level,
                desc->src_subresource.base_array_layer,
                desc->src_subresource.layer_count 
            },
            .srcOffset = { 0, 0, 0 },
            .dstSubresource = {                     //
                desc->dst_subresource.aspects,          //
                desc->dst_subresource.mip_level,        //
                desc->dst_subresource.base_array_layer, //
                desc->dst_subresource.layer_count 
            },
            .dstOffset = { 0, 0, 0 },
            .extent = { width, height, depth },
        };
        D->mVkDeviceTable.vkCmdCopyImage(Cmd->pVkCmdBuf,
        Src->pVkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        Dst->pVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    }
}

void cgpu_free_buffer_vulkan(CGPUBufferId buffer)
{
    CGPUBuffer_Vulkan* B = (CGPUBuffer_Vulkan*)buffer;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)B->super.device;
    cgpu_assert(B->pVkAllocation && "pVkAllocation must not be null!");
    if (B->pVkUniformTexelView)
    {
        vkDestroyBufferView(D->pVkDevice, B->pVkUniformTexelView, GLOBAL_VkAllocationCallbacks);
        B->pVkUniformTexelView = VK_NULL_HANDLE;
    }
    if (B->pVkStorageTexelView)
    {
        vkDestroyBufferView(D->pVkDevice, B->pVkUniformTexelView, GLOBAL_VkAllocationCallbacks);
        B->pVkStorageTexelView = VK_NULL_HANDLE;
    }
    vmaDestroyBuffer(D->pVmaAllocator, B->pVkBuffer, B->pVkAllocation);
    cgpu_free_aligned(B);
}

// Texture/TextureView APIs
cgpu_static_assert(sizeof(CGPUTexture_Vulkan) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line
typedef struct ImportHandleInfo {
    void* pHandle;
    VkExternalMemoryHandleTypeFlagBitsKHR mHandleType;
} ImportHandleInfo;
CGPUTextureId cgpu_create_texture_vulkan(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc)
{
    if (desc->sample_count > CGPU_SAMPLE_COUNT_1 && desc->mip_levels > 1)
    {
        cgpu_error("Multi-Sampled textures cannot have mip maps");
        cgpu_assert(false);
        return CGPU_NULLPTR;
    }
    // Alloc aligned memory
    size_t totalSize = sizeof(CGPUTexture_Vulkan);
    uint64_t unique_id = UINT64_MAX;
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)desc->owner_queue;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)device->adapter;

    bool owns_image = false;
    bool is_dedicated = false;
    bool can_alias_alloc = false;
    bool is_imported = false;

    VkImageType mImageType;
    VkImage pVkImage = VK_NULL_HANDLE;
    VkDeviceMemory pVkDeviceMemory = VK_NULL_HANDLE;
    uint32_t aspect_mask = 0;
    VmaAllocation vmaAllocation = VK_NULL_HANDLE;
    const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(desc->format);
    const CGPUFormatSupport* format_support = &A->adapter_detail.format_supports[desc->format];
    if (desc->native_handle && !(desc->flags & CGPU_INNER_TCF_IMPORT_SHARED_HANDLE))
    {
        owns_image = false;
        pVkImage = (VkImage)desc->native_handle;
    }
    else if (!desc->is_aliasing)
    {
        owns_image = true;
    }

    // Usage flags
    VkImageUsageFlags additionalFlags = 0;
    if (desc->descriptors & CGPU_RESOURCE_TYPE_RENDER_TARGET)
        additionalFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    else if (is_depth_stencil)
        additionalFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    uint32_t arraySize = desc->array_size;
    // Image type
    mImageType = VK_IMAGE_TYPE_MAX_ENUM;
    if (desc->flags & CGPU_TCF_FORCE_2D)
    {
        cgpu_assert(desc->depth == 1);
        mImageType = VK_IMAGE_TYPE_2D;
    }
    else if (desc->flags & CGPU_TCF_FORCE_3D)
        mImageType = VK_IMAGE_TYPE_3D;
    else
    {
        if (desc->depth > 1)
            mImageType = VK_IMAGE_TYPE_3D;
        else if (desc->height > 1)
            mImageType = VK_IMAGE_TYPE_2D;
        else
            mImageType = VK_IMAGE_TYPE_1D;
    }
    CGPUResourceTypes descriptors = desc->descriptors;
    bool cubemapRequired = (CGPU_RESOURCE_TYPE_TEXTURE_CUBE == (descriptors & CGPU_RESOURCE_TYPE_TEXTURE_CUBE));
    bool arrayRequired = mImageType == VK_IMAGE_TYPE_3D;
    // TODO: Support stencil format
    const bool isStencilFormat = false;
    (void)isStencilFormat;
    // TODO: Support planar format
    const bool isPlanarFormat = false;
    const uint32_t numOfPlanes = 1;
    const bool isSinglePlane = true;
    cgpu_assert(((isSinglePlane && numOfPlanes == 1) || (!isSinglePlane && numOfPlanes > 1 && numOfPlanes <= MAX_PLANE_COUNT)) &&
        "Number of planes for multi-planar formats must be 2 or 3 and for single-planar formats it must be 1.");

    if (pVkImage == VK_NULL_HANDLE)
    {
        VkImageCreateInfo imageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .imageType = mImageType,
            .format = (VkFormat)VkUtil_FormatTranslateToVk(desc->format),
            .extent.width = desc->width,
            .extent.height = desc->height,
            .extent.depth = desc->depth,
            .mipLevels = desc->mip_levels,
            .arrayLayers = arraySize,
            .samples = VkUtil_SampleCountTranslateToVk(desc->sample_count),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VkUtil_DescriptorTypesToImageUsage(descriptors),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = NULL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };
        aspect_mask = VkUtil_DeterminAspectMask(imageCreateInfo.format, true);
        imageCreateInfo.usage |= additionalFlags;
        if (cubemapRequired)
            imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        if (arrayRequired)
            imageCreateInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;

        DECLARE_ZERO(VkFormatProperties, format_props);
        vkGetPhysicalDeviceFormatProperties(A->pPhysicalDevice, imageCreateInfo.format, &format_props);
        if (isPlanarFormat) // multi-planar formats must have each plane separately bound to memory, rather than having a single memory binding for the whole image
        {
            cgpu_assert(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT);
            imageCreateInfo.flags |= VK_IMAGE_CREATE_DISJOINT_BIT;
        }
        if ((VK_IMAGE_USAGE_SAMPLED_BIT & imageCreateInfo.usage) || (VK_IMAGE_USAGE_STORAGE_BIT & imageCreateInfo.usage))
        {
            // Make it easy to copy to and from textures
            imageCreateInfo.usage |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        }
        cgpu_assert(format_support->shader_read && "GPU shader can't' read from this format");
        // Verify that GPU supports this format
        VkFormatFeatureFlags format_features = VkUtil_ImageUsageToFormatFeatures(imageCreateInfo.usage);
        VkFormatFeatureFlags flags = format_props.optimalTilingFeatures & format_features;
        cgpu_assert((flags != 0) && "Format is not supported for GPU local images (i.e. not host visible images)");
        DECLARE_ZERO(VmaAllocationCreateInfo, mem_reqs)
        if (desc->is_aliasing)
        {
            // Aliasing VkImage
            VkResult res = D->mVkDeviceTable.vkCreateImage(D->pVkDevice, &imageCreateInfo, GLOBAL_VkAllocationCallbacks, &pVkImage);
            CHECK_VKRESULT(res);
        }
        else
        {
            // Allocate texture memory
            if (desc->flags & CGPU_TCF_OWN_MEMORY_BIT)
                mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            mem_reqs.usage = (VmaMemoryUsage)VMA_MEMORY_USAGE_GPU_ONLY;
#ifdef USE_EXTERNAL_MEMORY_EXTENSIONS
            VkExternalMemoryImageCreateInfo externalInfo = { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR, NULL };
            VkExportMemoryAllocateInfo exportMemoryInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR, NULL };
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            wchar_t* win32Name = CGPU_NULLPTR;
            const wchar_t* nameFormat = L"cgpu-shared-texture-%llu";
            VkImportMemoryWin32HandleInfoKHR win32ImportInfo = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, NULL };
            VkExportMemoryWin32HandleInfoKHR win32ExportMemoryInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR, NULL };
#endif
            if (A->external_memory && (desc->flags & CGPU_INNER_TCF_IMPORT_SHARED_HANDLE))
            {
                is_imported = true;
                imageCreateInfo.pNext = &externalInfo;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                CGPUImportTextureDescriptor* pImportDesc = (CGPUImportTextureDescriptor*)desc->native_handle;
                externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
                if (pImportDesc->backend == CGPU_BACKEND_D3D12)
                {
                    externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
                    win32ImportInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
                }
                if (pImportDesc->backend == CGPU_BACKEND_VULKAN)
                {
                    externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
                    win32ImportInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
                }
                // format name wstring
                unique_id = pImportDesc->shared_handle;
                int size_needed = swprintf(CGPU_NULL, 0, nameFormat, unique_id);
                win32Name = cgpu_calloc(1 + size_needed, sizeof(wchar_t));
                swprintf(win32Name, 1 + size_needed, nameFormat, unique_id);
                // record import info
                win32ImportInfo.handle = NULL;
                win32ImportInfo.name = win32Name;
                // Allocate external (importable / exportable) memory as dedicated memory to avoid adding unnecessary complexity to the Vulkan Memory Allocator
                uint32_t memoryType = 0;
                VkResult findResult = vmaFindMemoryTypeIndexForImageInfo(D->pVmaAllocator, &imageCreateInfo, &mem_reqs, &memoryType);
                if (findResult != VK_SUCCESS)
                {
                    cgpu_error("Failed to find memory type for image");
                }
                // import memory
                VkResult importRes = D->mVkDeviceTable.vkCreateImage(D->pVkDevice, &imageCreateInfo, GLOBAL_VkAllocationCallbacks, &pVkImage);
                CHECK_VKRESULT(importRes);
                VkMemoryDedicatedRequirements MemoryDedicatedRequirements = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS };
                VkMemoryRequirements2 MemoryRequirements2 = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
                MemoryRequirements2.pNext = &MemoryDedicatedRequirements;
                VkImageMemoryRequirementsInfo2 ImageMemoryRequirementsInfo2 = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2 };
                ImageMemoryRequirementsInfo2.image = pVkImage;
                // WARN: Memory access violation unless validation instance layer is enabled, otherwise success but...
                D->mVkDeviceTable.vkGetImageMemoryRequirements2(D->pVkDevice, &ImageMemoryRequirementsInfo2, &MemoryRequirements2);
                VkMemoryAllocateInfo importAllocation = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .allocationSize = MemoryRequirements2.memoryRequirements.size, // this is valid for import allocations
                    .memoryTypeIndex = memoryType,
                    .pNext = &win32ImportInfo
                }; 
                cgpu_info("Importing external memory %ls allocation of size %llu", win32Name, importAllocation.allocationSize);
                importRes = D->mVkDeviceTable.vkAllocateMemory(D->pVkDevice, &importAllocation, GLOBAL_VkAllocationCallbacks, &pVkDeviceMemory);
                CHECK_VKRESULT(importRes);
                // bind memory
                importRes = D->mVkDeviceTable.vkBindImageMemory(D->pVkDevice, pVkImage, pVkDeviceMemory, 0);
                CHECK_VKRESULT(importRes);
                if (importRes == VK_SUCCESS)
                {
                    cgpu_trace("Imported image %p with allocation %p", pVkImage, pVkDeviceMemory);
                }
#endif
            }
            else if (A->external_memory && desc->flags & CGPU_TCF_EXPORT_BIT)
            {
                imageCreateInfo.pNext = &externalInfo;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                const VkExternalMemoryHandleTypeFlags exportFlags = 
                    VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT | 
                    VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT;
                externalInfo.handleTypes = exportFlags;
                // format name wstring
                uint64_t pid = (uint64_t)GetCurrentProcessId();
                uint64_t shared_id = D->next_shared_id++;
                unique_id = (pid << 32) | shared_id;
                int size_needed = swprintf(CGPU_NULL, 0, nameFormat, unique_id);
                win32Name = cgpu_calloc(1 + size_needed, sizeof(wchar_t));
                swprintf(win32Name, 1 + size_needed, nameFormat, unique_id);
                // record export info
                win32ExportMemoryInfo.dwAccess = GENERIC_ALL;
                win32ExportMemoryInfo.name = win32Name;
                win32ExportMemoryInfo.pAttributes = CGPU_NULLPTR;
                exportMemoryInfo.pNext = &win32ExportMemoryInfo;
                exportMemoryInfo.handleTypes = exportFlags;
                cgpu_trace("Exporting texture with name %ls size %dx%dx%d", win32Name, desc->width, desc->height, desc->depth);
#else
                exportMemoryInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
                // mem_reqs.pUserData = &exportMemoryInfo;
                uint32_t memoryType = 0;
                VkResult findResult = vmaFindMemoryTypeIndexForImageInfo(D->pVmaAllocator, &imageCreateInfo, &mem_reqs, &memoryType);
                if (findResult != VK_SUCCESS)
                {
                    cgpu_error("Failed to find memory type for image");
                }
                if (D->pExternalMemoryVmaPools[memoryType] == CGPU_NULLPTR)
                {
                    D->pExternalMemoryVmaPoolNexts[memoryType] = cgpu_calloc(1, sizeof(VkExportMemoryAllocateInfoKHR));
                    VkExportMemoryAllocateInfoKHR* Next = (VkExportMemoryAllocateInfoKHR*)D->pExternalMemoryVmaPoolNexts[memoryType];
                    Next->sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
                    VmaPoolCreateInfo poolCreateInfo = {
                        .memoryTypeIndex  = memoryType,
                        .blockSize = 0,
                        .maxBlockCount = 1024,
                        .pMemoryAllocateNext = D->pExternalMemoryVmaPoolNexts[memoryType]
                    };
                    if (vmaCreatePool(D->pVmaAllocator, &poolCreateInfo, &D->pExternalMemoryVmaPools[memoryType]) != VK_SUCCESS)
                    {
                        cgpu_assert(0 && "Failed to create VMA Pool");
                    }
                }
                memcpy(D->pExternalMemoryVmaPoolNexts[memoryType], &exportMemoryInfo, sizeof(VkExportMemoryAllocateInfoKHR));
                mem_reqs.pool = D->pExternalMemoryVmaPools[memoryType];
                // Allocate external (importable / exportable) memory as dedicated memory to avoid adding unnecessary complexity to the Vulkan Memory Allocator
                mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            }
#endif
            // Do allocation if is not imported
            if (!is_imported)
            {
                VmaAllocationInfo alloc_info = { 0 };
                if (isSinglePlane)
                {
                    if (!desc->is_dedicated && !is_imported && !(desc->flags & CGPU_TCF_EXPORT_BIT))
                    {
                        mem_reqs.flags |= VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
                    }
                    VkResult res = vmaCreateImage(D->pVmaAllocator,
                        &imageCreateInfo, &mem_reqs, &pVkImage,
                        &vmaAllocation, &alloc_info);
                    CHECK_VKRESULT(res);
                }
                else // Multi-planar formats
                {
                    // TODO: Planar formats
                }
            }
#ifdef VK_USE_PLATFORM_WIN32_KHR
            cgpu_free(win32Name);
#endif
            is_dedicated = mem_reqs.flags & VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            can_alias_alloc = mem_reqs.flags & VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
        }
    }
    CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)cgpu_calloc_aligned(1, totalSize, _Alignof(CGPUTexture_Vulkan));
    cgpu_assert(T);
    T->super.owns_image = owns_image;
    T->super.aspect_mask = aspect_mask;
    T->super.is_dedicated = is_dedicated;
    T->super.is_aliasing = desc->is_aliasing;
    T->super.can_alias = can_alias_alloc || desc->is_aliasing;
    T->pVkImage = pVkImage;
    if (pVkDeviceMemory) T->pVkDeviceMemory = pVkDeviceMemory;
    if (vmaAllocation) T->pVkAllocation = vmaAllocation;
    T->super.width = desc->width;
    T->super.height = desc->height;
    T->super.depth = desc->depth;
    T->super.mip_levels = desc->mip_levels;
    T->super.is_cube = cubemapRequired;
    T->super.array_size_minus_one = arraySize - 1;
    T->super.format = desc->format;
    T->super.is_imported = is_imported;
    T->super.unique_id = (unique_id == UINT64_MAX) ? D->super.next_texture_id++ : unique_id;
    // Set Texture Name
    VkUtil_OptionalSetObjectName(D, (uint64_t)T->pVkImage, VK_OBJECT_TYPE_IMAGE, desc->name);
    // Start state
    if (Q && T->pVkImage != VK_NULL_HANDLE && T->pVkAllocation != VK_NULL_HANDLE)
    {
#ifdef CGPU_THREAD_SAFETY
        if (Q->pMutex) skr_acquire_mutex(Q->pMutex);
#endif
        cgpu_reset_command_pool(Q->pInnerCmdPool);
        cgpu_cmd_begin(Q->pInnerCmdBuffer);
        CGPUTextureBarrier init_barrier = {
            .texture = &T->super,
            .src_state = CGPU_RESOURCE_STATE_UNDEFINED,
            .dst_state = desc->start_state
        };
        CGPUResourceBarrierDescriptor init_barrier_d = {
            .texture_barriers = &init_barrier,
            .texture_barriers_count = 1
        };
        cgpu_cmd_resource_barrier(Q->pInnerCmdBuffer, &init_barrier_d);
        cgpu_cmd_end(Q->pInnerCmdBuffer);
        CGPUQueueSubmitDescriptor barrier_submit = {
            .cmds = &Q->pInnerCmdBuffer,
            .cmds_count = 1,
            .signal_fence = Q->pInnerFence
        };
        cgpu_submit_queue(&Q->super, &barrier_submit);
        cgpu_wait_fences(&Q->pInnerFence, 1);
#ifdef CGPU_THREAD_SAFETY
        if (Q->pMutex) skr_release_mutex(Q->pMutex);
#endif
    }
    return &T->super;
}

#ifdef _WIN32
extern uint64_t cgpu_export_shared_texture_handle_vulkan_win32(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc);
extern CGPUTextureId cgpu_import_shared_texture_handle_vulkan_win32(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc);
#endif

uint64_t cgpu_export_shared_texture_handle_vulkan(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc)
{
#ifdef _WIN32
    return cgpu_export_shared_texture_handle_vulkan_win32(device, desc);
#endif
    return UINT64_MAX;
}

CGPUTextureId cgpu_import_shared_texture_handle_vulkan(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc)
{
#ifdef _WIN32
    return cgpu_import_shared_texture_handle_vulkan_win32(device, desc);
#endif
    return CGPU_NULLPTR;
}

void cgpu_free_texture_vulkan(CGPUTextureId texture)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)texture->device;
    CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)texture;
    if (T->pVkImage != VK_NULL_HANDLE)
    {
        if (T->super.is_imported)
        {
            D->mVkDeviceTable.vkDestroyImage(D->pVkDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
            D->mVkDeviceTable.vkFreeMemory(D->pVkDevice, T->pVkDeviceMemory, GLOBAL_VkAllocationCallbacks);
        }
        else if (T->super.owns_image)
        {
            const ECGPUFormat fmt = texture->format;
            (void)fmt;
            // TODO: Support planar formats
            const bool isSinglePlane = true;
            if (isSinglePlane)
            {
                cgpu_trace("Freeing texture allocation %p \n\t size: %dx%dx%d owns_image: %d imported: %d", 
                    T->pVkImage, texture->width, texture->height, texture->depth, T->super.owns_image, T->super.is_imported);

                vmaDestroyImage(D->pVmaAllocator, T->pVkImage, T->pVkAllocation);
            }
            else
            {
                D->mVkDeviceTable.vkDestroyImage(D->pVkDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
                D->mVkDeviceTable.vkFreeMemory(D->pVkDevice, T->pVkDeviceMemory, GLOBAL_VkAllocationCallbacks);
            }
        }
        else
        {
            cgpu_trace("Freeing texture %p \n\t size: %dx%dx%d owns_image: %d imported: %d", 
                    T->pVkImage, texture->width, texture->height, texture->depth, T->super.owns_image, T->super.is_imported);

            D->mVkDeviceTable.vkDestroyImage(D->pVkDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
        }
    }
    cgpu_free_aligned(T);
}

CGPUTextureViewId cgpu_create_texture_view_vulkan(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)desc->texture->device;
    CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)desc->texture;
    CGPUTextureView_Vulkan* TV = (CGPUTextureView_Vulkan*)cgpu_calloc_aligned(1, sizeof(CGPUTextureView_Vulkan), _Alignof(CGPUTextureView_Vulkan));
    VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    VkImageType mImageType = T->super.is_cube ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    switch (mImageType)
    {
        case VK_IMAGE_TYPE_1D:
            view_type = desc->array_layer_count > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;
        case VK_IMAGE_TYPE_2D:
            if (T->super.is_cube)
                view_type = (desc->dims == CGPU_TEX_DIMENSION_CUBE_ARRAY) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
            else
                view_type = ((desc->dims == CGPU_TEX_DIMENSION_2D_ARRAY) || (desc->dims == CGPU_TEX_DIMENSION_2DMS_ARRAY)) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            break;
        case VK_IMAGE_TYPE_3D:
            if (desc->array_layer_count > 1)
            {
                cgpu_error("Cannot support 3D Texture Array in Vulkan");
                cgpu_assert(false);
            }
            view_type = VK_IMAGE_VIEW_TYPE_3D;
            break;
        default:
            cgpu_assert(false && "Image Format not supported!");
            break;
    }
    cgpu_assert(view_type != VK_IMAGE_VIEW_TYPE_MAX_ENUM && "Invalid Image View");

    // Determin aspect mask
    VkImageAspectFlags aspectMask = 0;
    if (desc->aspects & CGPU_TVA_STENCIL)
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    if (desc->aspects & CGPU_TVA_COLOR)
        aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    if (desc->aspects & CGPU_TVA_DEPTH)
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

    // SRV
    VkImageViewCreateInfo srvDesc = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .image = T->pVkImage,
        .viewType = view_type,
        .format = VkUtil_FormatTranslateToVk(desc->format),
        .components.r = VK_COMPONENT_SWIZZLE_R,
        .components.g = VK_COMPONENT_SWIZZLE_G,
        .components.b = VK_COMPONENT_SWIZZLE_B,
        .components.a = VK_COMPONENT_SWIZZLE_A,
        .subresourceRange.aspectMask = aspectMask,
        .subresourceRange.baseMipLevel = desc->base_mip_level,
        .subresourceRange.levelCount = desc->mip_level_count,
        .subresourceRange.baseArrayLayer = desc->base_array_layer,
        .subresourceRange.layerCount = desc->array_layer_count
    };
    if (desc->usages & CGPU_TVU_SRV)
    {
        CHECK_VKRESULT(D->mVkDeviceTable.vkCreateImageView(D->pVkDevice, &srvDesc, GLOBAL_VkAllocationCallbacks, &TV->pVkSRVDescriptor));
    }
    // UAV
    if (desc->usages & CGPU_TVU_UAV)
    {
        VkImageViewCreateInfo uavDesc = srvDesc;
        // #NOTE : We dont support imageCube, imageCubeArray for consistency with other APIs
        // All cubemaps will be used as image2DArray for Image Load / Store ops
        if (uavDesc.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY || uavDesc.viewType == VK_IMAGE_VIEW_TYPE_CUBE)
            uavDesc.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        uavDesc.subresourceRange.baseMipLevel = desc->base_mip_level;
        CHECK_VKRESULT(D->mVkDeviceTable.vkCreateImageView(D->pVkDevice, &uavDesc, GLOBAL_VkAllocationCallbacks, &TV->pVkUAVDescriptor));
    }
    // RTV & DSV
    if (desc->usages & CGPU_TVU_RTV_DSV)
    {
        CHECK_VKRESULT(D->mVkDeviceTable.vkCreateImageView(D->pVkDevice, &srvDesc, GLOBAL_VkAllocationCallbacks, &TV->pVkRTVDSVDescriptor));
    }
    return &TV->super;
}

void cgpu_free_texture_view_vulkan(CGPUTextureViewId render_target)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)render_target->device;
    CGPUTextureView_Vulkan* TV = (CGPUTextureView_Vulkan*)render_target;
    // Free descriptors
    if (VK_NULL_HANDLE != TV->pVkSRVDescriptor)
        D->mVkDeviceTable.vkDestroyImageView(D->pVkDevice, TV->pVkSRVDescriptor, GLOBAL_VkAllocationCallbacks);
    if (VK_NULL_HANDLE != TV->pVkRTVDSVDescriptor)
        D->mVkDeviceTable.vkDestroyImageView(D->pVkDevice, TV->pVkRTVDSVDescriptor, GLOBAL_VkAllocationCallbacks);
    if (VK_NULL_HANDLE != TV->pVkUAVDescriptor)
        D->mVkDeviceTable.vkDestroyImageView(D->pVkDevice, TV->pVkUAVDescriptor, GLOBAL_VkAllocationCallbacks);
    cgpu_free_aligned(TV);
}

bool cgpu_try_bind_aliasing_texture_vulkan(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    if (desc->aliased)
    {
        CGPUTexture_Vulkan* Aliased = (CGPUTexture_Vulkan*)desc->aliased;
        CGPUTexture_Vulkan* Aliasing = (CGPUTexture_Vulkan*)desc->aliasing;
        cgpu_assert(Aliasing->super.is_aliasing && "aliasing texture need to be created as aliasing!");
        if (Aliased->pVkImage != VK_NULL_HANDLE &&
            Aliased->pVkAllocation != VK_NULL_HANDLE &&
            Aliasing->pVkImage != VK_NULL_HANDLE &&
            !Aliased->super.is_dedicated &&
            Aliasing->super.is_aliasing)
        {
            VkMemoryRequirements aliasingMemReq;
            VkMemoryRequirements aliasedMemReq;
            D->mVkDeviceTable.vkGetImageMemoryRequirements(D->pVkDevice,
            Aliasing->pVkImage, &aliasingMemReq);
            D->mVkDeviceTable.vkGetImageMemoryRequirements(D->pVkDevice,
            Aliased->pVkImage, &aliasedMemReq);
            if (aliasedMemReq.size >= aliasingMemReq.size &&
                aliasedMemReq.alignment >= aliasingMemReq.alignment &&
                aliasedMemReq.memoryTypeBits & aliasingMemReq.memoryTypeBits)
            {
                const bool isSinglePlane = true;
                if (isSinglePlane)
                {
                    VkResult res = vmaBindImageMemory(D->pVmaAllocator,
                    Aliased->pVkAllocation, Aliasing->pVkImage);
                    if (res == VK_SUCCESS)
                    {
                        Aliasing->pVkAllocation = Aliased->pVkAllocation;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Sampler APIs
CGPUSamplerId cgpu_create_sampler_vulkan(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUSampler_Vulkan* S = (CGPUSampler_Vulkan*)cgpu_calloc_aligned(1, sizeof(CGPUSampler_Vulkan), _Alignof(CGPUSampler_Vulkan));
    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .magFilter = VkUtil_TranslateFilterType(desc->mag_filter),
        .minFilter = VkUtil_TranslateFilterType(desc->min_filter),
        .mipmapMode = VkUtil_TranslateMipMapMode(desc->mipmap_mode),
        .addressModeU = VkUtil_TranslateAddressMode(desc->address_u),
        .addressModeV = VkUtil_TranslateAddressMode(desc->address_v),
        .addressModeW = VkUtil_TranslateAddressMode(desc->address_w),
        .mipLodBias = desc->mip_lod_bias,
        .anisotropyEnable = (desc->max_anisotropy > 0.0f) ? VK_TRUE : VK_FALSE,
        .maxAnisotropy = desc->max_anisotropy,
        .compareEnable = (gVkComparisonFuncTranslator[desc->compare_func] != VK_COMPARE_OP_NEVER) ? VK_TRUE : VK_FALSE,
        .compareOp = gVkComparisonFuncTranslator[desc->compare_func],
        .minLod = 0.0f,
        .maxLod = ((desc->mipmap_mode == CGPU_MIPMAP_MODE_LINEAR) ? FLT_MAX : 0.0f),
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateSampler(D->pVkDevice, &sampler_info, GLOBAL_VkAllocationCallbacks, &S->pVkSampler));
    return &S->super;
}

void cgpu_free_sampler_vulkan(CGPUSamplerId sampler)
{
    CGPUSampler_Vulkan* S = (CGPUSampler_Vulkan*)sampler;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)sampler->device;
    D->mVkDeviceTable.vkDestroySampler(D->pVkDevice, S->pVkSampler, GLOBAL_VkAllocationCallbacks);
    cgpu_free_aligned(S);
}

// Shader APIs
CGPUShaderLibraryId cgpu_create_shader_library_vulkan(
CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = desc->code_size,
        .pCode = desc->code
    };
    CGPUShaderLibrary_Vulkan* S = (CGPUShaderLibrary_Vulkan*)cgpu_calloc(1, sizeof(CGPUShaderLibrary_Vulkan));
    D->mVkDeviceTable.vkCreateShaderModule(D->pVkDevice, &info, GLOBAL_VkAllocationCallbacks, &S->mShaderModule);
    VkUtil_InitializeShaderReflection(device, S, desc);
    return &S->super;
}

void cgpu_free_shader_library_vulkan(CGPUShaderLibraryId library)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)library->device;
    CGPUShaderLibrary_Vulkan* S = (CGPUShaderLibrary_Vulkan*)library;
    VkUtil_FreeShaderReflection(S);
    D->mVkDeviceTable.vkDestroyShaderModule(D->pVkDevice, S->mShaderModule, GLOBAL_VkAllocationCallbacks);
    cgpu_free(S);
}