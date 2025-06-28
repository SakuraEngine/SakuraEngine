#include "SkrBase/atomic/atomic.h"
#include "SkrGraphics/api.h"
#include "SkrGraphics/backend/vulkan/cgpu_vulkan.h"
#include "../common/common_utils.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_utils.h"
#ifdef CGPU_THREAD_SAFETY
    #include "SkrOS/thread.h"
#endif
#include <string.h>

SKR_FORCEINLINE static VkBufferCreateInfo VkUtil_CreateBufferCreateInfo(CGPUAdapter_Vulkan* A, const struct CGPUBufferDescriptor* desc)
{
    uint64_t allocationSize = desc->size;
    // Align the buffer size to multiples of the dynamic uniform buffer minimum size
    if (desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
    {
        uint64_t minAlignment = A->adapter_detail.uniform_buffer_alignment;
        allocationSize = cgpu_round_up(allocationSize, minAlignment);
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

SKR_FORCEINLINE static VkFormatFeatureFlags VkUtil_ImageUsageToFormatFeatures(VkImageUsageFlags usage)
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
    if (desc->flags & CGPU_BCF_DEDICATED_BIT)
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
    SKR_DECLARE_ZERO(VmaAllocationInfo, alloc_info)
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
    CGPUBuffer_Vulkan* B = cgpu_calloc_aligned(1, sizeof(CGPUBuffer_Vulkan) + sizeof(CGPUBufferInfo), _Alignof(CGPUBuffer_Vulkan));
    CGPUBufferInfo* info = (CGPUBufferInfo*)(B + 1);
    B->super.info = info;
    B->pVkAllocation = mVmaAllocation;
    B->pVkBuffer = pVkBuffer;

    // Set Buffer Object Props
    info->size = desc->size;
    info->cpu_mapped_address = alloc_info.pMappedData;
    info->memory_usage = desc->memory_usage;
    info->descriptors = desc->descriptors;

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
        SKR_DECLARE_ZERO(VkFormatProperties, formatProps)
        vkGetPhysicalDeviceFormatProperties(A->pPhysicalDevice, texel_format, &formatProps);
        // Now We Use The Same View Info for Uniform & Storage BufferView on Vulkan Backend.
        VkBufferViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .pNext = NULL,
            .buffer = B->pVkBuffer,
            .flags = 0,
            .format = texel_format,
            .offset = desc->first_element * desc->element_stride,
            .range = desc->element_count * desc->element_stride
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

    // Start state
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)desc->owner_queue;
    if (Q && B->pVkBuffer != VK_NULL_HANDLE && B->pVkAllocation != VK_NULL_HANDLE)
    {
#ifdef CGPU_THREAD_SAFETY
        if (Q->pMutex) skr_mutex_acquire(Q->pMutex);
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
        if (Q->pMutex) skr_mutex_release(Q->pMutex);
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
        cgpu_assert(buffer->info->memory_usage != CGPU_MEM_USAGE_GPU_ONLY && "Trying to map non-cpu accessible resource");

    CGPUBufferInfo* pInfo = (CGPUBufferInfo*)buffer->info;
    VkResult vk_res = vmaMapMemory(D->pVmaAllocator, B->pVkAllocation, &pInfo->cpu_mapped_address);
    cgpu_assert(vk_res == VK_SUCCESS);

    if (range && (vk_res == VK_SUCCESS))
    {
        pInfo->cpu_mapped_address = ((uint8_t*)pInfo->cpu_mapped_address + range->offset);
    }
}

void cgpu_unmap_buffer_vulkan(CGPUBufferId buffer)
{
    CGPUBuffer_Vulkan* B = (CGPUBuffer_Vulkan*)buffer;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)B->super.device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)buffer->device->adapter;
    if (!A->adapter_detail.support_host_visible_vram)
        cgpu_assert(buffer->info->memory_usage != CGPU_MEM_USAGE_GPU_ONLY && "Trying to unmap non-cpu accessible resource");

    CGPUBufferInfo* pInfo = (CGPUBufferInfo*)buffer->info;
    vmaUnmapMemory(D->pVmaAllocator, B->pVkAllocation);
    pInfo->cpu_mapped_address = CGPU_NULLPTR;
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
    const CGPUTextureInfo* texInfo = desc->dst->info;
    const ECGPUFormat fmt = texInfo->format;
    if (isSinglePlane)
    {
        const uint64_t width = cgpu_max(1, texInfo->width >> desc->dst_subresource.mip_level);
        const uint64_t height = cgpu_max(1, texInfo->height >> desc->dst_subresource.mip_level);
        const uint64_t depth = cgpu_max(1, texInfo->depth >> desc->dst_subresource.mip_level);

		const uint64_t xBlocksCount = width / FormatUtil_WidthOfBlock(fmt);
		const uint64_t yBlocksCount = height / FormatUtil_HeightOfBlock(fmt);

        VkBufferImageCopy copy = {
            .bufferOffset = desc->src_offset,
            .bufferRowLength = (uint32_t)xBlocksCount * FormatUtil_WidthOfBlock(fmt),
            .bufferImageHeight = (uint32_t)yBlocksCount * FormatUtil_HeightOfBlock(fmt),
            .imageSubresource.aspectMask = (VkImageAspectFlags)texInfo->aspect_mask,
            .imageSubresource.mipLevel = desc->dst_subresource.mip_level,
            .imageSubresource.baseArrayLayer = desc->dst_subresource.base_array_layer,
            .imageSubresource.layerCount = desc->dst_subresource.layer_count,
            .imageOffset.x = 0,
            .imageOffset.y = 0,
            .imageOffset.z = 0,
            .imageExtent.width = (uint32_t)width,
            .imageExtent.height = (uint32_t)height,
            .imageExtent.depth = (uint32_t)depth
        };
        D->mVkDeviceTable.vkCmdCopyBufferToImage(Cmd->pVkCmdBuf,
            Src->pVkBuffer, Dst->pVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
            &copy);
    }
}

void cgpu_cmd_transfer_buffer_to_tiles_vulkan(CGPUCommandBufferId cmd, const struct CGPUBufferToTilesTransfer* desc)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    const CGPUTexture_Vulkan* Dst = (const CGPUTexture_Vulkan*)desc->dst;
    CGPUBuffer_Vulkan* Src = (CGPUBuffer_Vulkan*)desc->src;

    const CGPUTiledTextureInfo* pTiledInfo = Dst->super.tiled_resource;
    const uint32_t TileWidth = pTiledInfo->tile_width_in_texels;
    const uint32_t TileHeight = pTiledInfo->tile_height_in_texels;
    const uint32_t TileDepth = pTiledInfo->tile_depth_in_texels;
    const uint64_t TileByteSize = pTiledInfo->tile_size;

    const VkFormat fmt = VkUtil_FormatTranslateToVk(Dst->super.info->format);
    const VkImageAspectFlags aspects = VkUtil_DeterminAspectMask(fmt, false);

    const CGPUCoordinate start = desc->region.start;
    const CGPUCoordinate end = desc->region.end;
    VkDeviceSize Offset = desc->src_offset;
    for (uint32_t z = start.y; z < end.z; z++)
        for (uint32_t y = start.y; y < end.y; y++)
            for (uint32_t x = start.x; x < end.x; x++)
            {
                VkBufferImageCopy copy = {
                    .bufferOffset = Offset,
                    .bufferRowLength = (uint32_t)TileWidth,
                    .bufferImageHeight = (uint32_t)pTiledInfo->tile_height_in_texels,
                    .imageSubresource.aspectMask = aspects,
                    .imageSubresource.mipLevel = desc->region.mip_level,
                    .imageSubresource.baseArrayLayer = desc->region.layer,
                    .imageSubresource.layerCount = 1,
                    .imageOffset.x = x * TileWidth,
                    .imageOffset.y = y * TileHeight,
                    .imageOffset.z = z * TileDepth,
                    .imageExtent.width = TileWidth,
                    .imageExtent.height = TileHeight,
                    .imageExtent.depth = TileDepth
                };
                D->mVkDeviceTable.vkCmdCopyBufferToImage(Cmd->pVkCmdBuf,
                    Src->pVkBuffer, Dst->pVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                    1, &copy);
                Offset += TileByteSize;
            }
}

void cgpu_cmd_transfer_texture_to_texture_vulkan(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUTexture_Vulkan* Dst = (CGPUTexture_Vulkan*)desc->dst;
    CGPUTexture_Vulkan* Src = (CGPUTexture_Vulkan*)desc->src;
    const bool isSinglePlane = true;
    const CGPUTextureInfo* texInfo = desc->dst->info;
    if (isSinglePlane)
    {
        const uint32_t width = (uint32_t)cgpu_max(1, texInfo->width >> desc->dst_subresource.mip_level);
        const uint32_t height = (uint32_t)cgpu_max(1, texInfo->height >> desc->dst_subresource.mip_level);
        const uint32_t depth = (uint32_t)cgpu_max(1, texInfo->depth >> desc->dst_subresource.mip_level);

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

void cgpu_cmd_fill_buffer_vulkan(CGPUCommandBufferId cmd, CGPUBufferId buffer, const struct CGPUFillBufferDescriptor* desc)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUBuffer_Vulkan* Buffer = (CGPUBuffer_Vulkan*)buffer;
    CGPUDevice_Vulkan* Device = (CGPUDevice_Vulkan*)buffer->device;
    PFN_vkCmdFillBuffer write_fn = Device->mVkDeviceTable.vkCmdFillBuffer;
    if (write_fn)
    {
        write_fn(Cmd->pVkCmdBuf, Buffer->pVkBuffer, desc->offset, sizeof(uint32_t), desc->value);
    }
}

void cgpu_cmd_fill_buffer_n_vulkan(CGPUCommandBufferId cmd, CGPUBufferId buffer, const struct CGPUFillBufferDescriptor* desc, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        cgpu_cmd_fill_buffer_vulkan(cmd, buffer, desc);
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
    cgpu_free_aligned(B, _Alignof(CGPUBuffer_Vulkan));
}

// Texture/TextureView APIs
cgpu_static_assert(sizeof(CGPUTexture_Vulkan) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line

VkImageType VkUtil_TranslateImageType(const struct CGPUTextureDescriptor* desc)
{
    VkImageType mImageType = VK_IMAGE_TYPE_MAX_ENUM;
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
    return mImageType;
}

#if defined(USE_EXTERNAL_MEMORY_EXTENSIONS) && defined(VK_USE_PLATFORM_WIN32_KHR)
void VkUtil_ImportSharedTexture(CGPUQueue_Vulkan* Q, VmaAllocationCreateInfo* pMemReq, VkImageCreateInfo* pImageCreateInfo, 
    const struct CGPUTextureDescriptor* desc, const wchar_t* win32Name, 
    VkExternalMemoryImageCreateInfo* pExternalInfo, VkImportMemoryWin32HandleInfoKHR* pWin32ImportInfo,
    VkImage* outImage, VkDeviceMemory* outMemory)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)Q->super.device;
    pImageCreateInfo->pNext = pExternalInfo;
    CGPUImportTextureDescriptor* pImportDesc = (CGPUImportTextureDescriptor*)desc->native_handle;
    pExternalInfo->handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if (pImportDesc->backend == CGPU_BACKEND_D3D12)
    {
        pExternalInfo->handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
        pWin32ImportInfo->handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
    }
    if (pImportDesc->backend == CGPU_BACKEND_VULKAN)
    {
        pExternalInfo->handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
        pWin32ImportInfo->handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    }

    // record import info
    pWin32ImportInfo->handle = NULL;
    pWin32ImportInfo->name = win32Name;
    // Allocate external (importable / exportable) memory as dedicated memory to avoid adding unnecessary complexity to the Vulkan Memory Allocator
    uint32_t memoryType = 0;
    VkResult findResult = vmaFindMemoryTypeIndexForImageInfo(D->pVmaAllocator, 
        pImageCreateInfo, pMemReq, &memoryType);
    if (findResult != VK_SUCCESS)
    {
        cgpu_error("Failed to find memory type for image");
    }
    // import memory
    VkResult importRes = D->mVkDeviceTable.vkCreateImage(D->pVkDevice, pImageCreateInfo, GLOBAL_VkAllocationCallbacks, outImage);
    CHECK_VKRESULT(importRes);
    VkMemoryDedicatedRequirements MemoryDedicatedRequirements = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS };
    VkMemoryRequirements2 MemoryRequirements2 = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
    MemoryRequirements2.pNext = &MemoryDedicatedRequirements;
    VkImageMemoryRequirementsInfo2 ImageMemoryRequirementsInfo2 = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2 };
    ImageMemoryRequirementsInfo2.image = *outImage;
    // WARN: Memory access violation unless validation instance layer is enabled, otherwise success but...
    D->mVkDeviceTable.vkGetImageMemoryRequirements2(D->pVkDevice, &ImageMemoryRequirementsInfo2, &MemoryRequirements2);
    VkMemoryAllocateInfo importAllocation = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = MemoryRequirements2.memoryRequirements.size, // this is valid for import allocations
        .memoryTypeIndex = memoryType,
        .pNext = pWin32ImportInfo
    }; 
    cgpu_info("Importing external memory %ls allocation of size %llu", win32Name, importAllocation.allocationSize);
    importRes = D->mVkDeviceTable.vkAllocateMemory(D->pVkDevice, &importAllocation, GLOBAL_VkAllocationCallbacks, outMemory);
    CHECK_VKRESULT(importRes);
    // bind memory
    importRes = D->mVkDeviceTable.vkBindImageMemory(D->pVkDevice, *outImage, *outMemory, 0);
    CHECK_VKRESULT(importRes);
    if (importRes == VK_SUCCESS)
    {
        cgpu_trace("Imported image %p with allocation %p", *outImage, *outMemory);
    }
}

void VkUtil_AllocateSharedTexture(CGPUQueue_Vulkan* Q, VmaAllocationCreateInfo* pMemReq, VkImageCreateInfo* pImageCreateInfo, 
    const struct CGPUTextureDescriptor* desc, const wchar_t* win32Name,
    VkExternalMemoryImageCreateInfo* pExternalInfo, VkExportMemoryAllocateInfo* pExportMemoryInfo, 
    VkExportMemoryWin32HandleInfoKHR* pWin32ExportMemoryInfo)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)Q->super.device;
    pImageCreateInfo->pNext = pExternalInfo;
    const VkExternalMemoryHandleTypeFlags exportFlags = 
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT | 
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT;
    pExternalInfo->handleTypes = exportFlags;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    // record export info
    pWin32ExportMemoryInfo->dwAccess = GENERIC_ALL;
    pWin32ExportMemoryInfo->name = win32Name;
    pWin32ExportMemoryInfo->pAttributes = CGPU_NULLPTR;
    pExportMemoryInfo->pNext = pWin32ExportMemoryInfo;
    pExportMemoryInfo->handleTypes = exportFlags;
    cgpu_trace("Exporting texture with name %ls size %dx%dx%d", win32Name, desc->width, desc->height, desc->depth);
#else
    exportMemoryInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

    // mem_reqs.pUserData = &exportMemoryInfo;
    uint32_t memoryType = 0;
    VkResult findResult = vmaFindMemoryTypeIndexForImageInfo(D->pVmaAllocator, pImageCreateInfo, pMemReq, &memoryType);
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
    memcpy(D->pExternalMemoryVmaPoolNexts[memoryType], pExportMemoryInfo, sizeof(VkExportMemoryAllocateInfoKHR));
    pMemReq->pool = D->pExternalMemoryVmaPools[memoryType];
    // Allocate external (importable / exportable) memory as dedicated memory to avoid adding unnecessary complexity to the Vulkan Memory Allocator
    pMemReq->flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
}
#endif

VkSparseImageMemoryRequirements VkUtil_FillTiledTextureInfo(CGPUDevice_Vulkan* D, CGPUTexture_Vulkan* T, const struct CGPUTextureDescriptor* desc, uint32_t* outTypeBits)
{
    VkImage pVkImage = T->pVkImage;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)D->super.adapter;
    // Get memory requirements
    VkMemoryRequirements sparseImageMemoryReqs;
    // Sparse image memory requirement counts
    D->mVkDeviceTable.vkGetImageMemoryRequirements(D->pVkDevice, pVkImage, &sparseImageMemoryReqs);
    // Check requested image size against hardware sparse limit
    if (sparseImageMemoryReqs.size > A->mPhysicalDeviceProps.properties.limits.sparseAddressSpaceSize)
    {
        cgpu_error("Requested sparse image size exceeds supported sparse address space size!");
    }
    // Get sparse memory requirements
    // Count
    uint32_t sparseMemoryReqsCount;
    bool colorAspectFound = false;
    bool noneStandardLayoutFound = false;
    D->mVkDeviceTable.vkGetImageSparseMemoryRequirements(D->pVkDevice, pVkImage, &sparseMemoryReqsCount, NULL);  // Get count
    SKR_DECLARE_ZERO(VkSparseImageMemoryRequirements, sparseReq);
    SKR_DECLARE_ZERO_VLA(VkSparseImageMemoryRequirements, sparseMemoryReqs, sparseMemoryReqsCount);
    if (sparseMemoryReqsCount == 0)
    {
        cgpu_error("No memory requirements for the sparse image!");
    }
    else
    {
        D->mVkDeviceTable.vkGetImageSparseMemoryRequirements(D->pVkDevice, pVkImage, &sparseMemoryReqsCount, sparseMemoryReqs);  // Get reqs
    }
    for (uint32_t i = 0; i < sparseMemoryReqsCount; i++)
    {
        if (sparseMemoryReqs[i].formatProperties.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
		{
            if (sparseMemoryReqs[i].formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT)
            {
                noneStandardLayoutFound = true;
            }
            else
            {
                sparseReq = sparseMemoryReqs[i];
                colorAspectFound = true;
                break;
            }
		}
    }
    if (!colorAspectFound)
    {
        if (noneStandardLayoutFound)
        {
            cgpu_error("Only found non-standard sparse block size layout!");
        }
        cgpu_error("Could not find sparse image memory requirements for color aspect bit!");
    }

    // iterate all mips and calculate the total number of tiles
    uint32_t layers = 1;
    uint32_t subres_cnt = layers * desc->mip_levels;
    uint32_t total_tiles_count = 0;

    CGPUTiledTextureInfo* pTiledInfo = cgpu_calloc_aligned(1, 
        sizeof(CGPUTiledTextureInfo) + subres_cnt * sizeof(CGPUTiledSubresourceInfo), 
        _Alignof(CGPUTiledTextureInfo));
    CGPUTiledSubresourceInfo* pSubresInfos = (CGPUTiledSubresourceInfo*)(pTiledInfo + 1);
    // record subresource infos
    for (uint32_t layer = 0; layer < 1; layer++)
    {
        for (uint32_t mip = 0; mip < desc->mip_levels; mip++)
        {
            uint32_t i = layer * desc->mip_levels + mip;
            const uint32_t mipWidth = (uint32_t)cgpu_max(1, desc->width >> mip);
            const uint32_t mipHeight = (uint32_t)cgpu_max(1, desc->height >> mip);
            const uint32_t mipDepth = (uint32_t)cgpu_max(1, desc->depth >> mip);
            const uint32_t mipWidthInTiles = (mipWidth + sparseReq.formatProperties.imageGranularity.width - 1) / sparseReq.formatProperties.imageGranularity.width;
            const uint32_t mipHeightInTiles = (mipHeight + sparseReq.formatProperties.imageGranularity.height - 1) / sparseReq.formatProperties.imageGranularity.height;
            const uint32_t mipDepthInTiles = (mipDepth + sparseReq.formatProperties.imageGranularity.depth - 1) / sparseReq.formatProperties.imageGranularity.depth;
            const uint32_t mipSizeInTiles = mipWidthInTiles * mipHeightInTiles * mipDepthInTiles;
            total_tiles_count += mipSizeInTiles;

            pSubresInfos[i].layer = layer;
            pSubresInfos[i].mip_level = mip;
            pSubresInfos[i].depth_in_tiles = mipDepthInTiles;
            pSubresInfos[i].height_in_tiles = mipHeightInTiles;
            pSubresInfos[i].width_in_tiles = mipWidthInTiles;
        }
    }

    SKR_ASSERT(sparseImageMemoryReqs.size == VK_SPARSE_PAGE_STANDARD_SIZE * total_tiles_count && "Unexpected sparse image memory requirements size!");
    pTiledInfo->tile_size = VK_SPARSE_PAGE_STANDARD_SIZE;
    pTiledInfo->total_tiles_count = total_tiles_count;
    pTiledInfo->alive_tiles_count = 0;
    pTiledInfo->alive_pack_count = 0;

    pTiledInfo->tile_width_in_texels = sparseReq.formatProperties.imageGranularity.width;
    pTiledInfo->tile_height_in_texels = sparseReq.formatProperties.imageGranularity.height;
    pTiledInfo->tile_depth_in_texels = sparseReq.formatProperties.imageGranularity.depth;
    pTiledInfo->subresources = pSubresInfos;

    pTiledInfo->packed_mip_start = sparseReq.imageMipTailFirstLod;
    pTiledInfo->packed_mip_count = desc->mip_levels - sparseReq.imageMipTailFirstLod;
    pTiledInfo->pack_unaligned = (sparseReq.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT) ? true : false;

    *outTypeBits = sparseImageMemoryReqs.memoryTypeBits;
    
    T->super.tiled_resource = pTiledInfo;
    
    return sparseReq;
}

CGPUTextureId cgpu_create_texture_vulkan(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc)
{
    if (desc->sample_count > CGPU_SAMPLE_COUNT_1 && desc->mip_levels > 1)
    {
        cgpu_error("Multi-Sampled textures cannot have mip maps");
        cgpu_assert(false);
        return CGPU_NULLPTR;
    }
    // Alloc aligned memory
    size_t totalSize = sizeof(CGPUTexture_Vulkan) + sizeof(CGPUTextureInfo);
    uint64_t unique_id = UINT64_MAX;
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)desc->owner_queue;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)device->adapter;

    bool owns_image = false;
    bool is_allocation_dedicated = false;
    bool can_alias_alloc = false;
    bool is_imported = false;

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
    else if (!(desc->flags & CGPU_TCF_ALIASING_RESOURCE))
    {
        owns_image = true;
    }

    uint32_t arraySize = desc->array_size;
    // Image type
    VkImageType mImageType = VkUtil_TranslateImageType(desc);
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
            .extent.width = (uint32_t)desc->width,
            .extent.height = (uint32_t)desc->height,
            .extent.depth = (uint32_t)desc->depth,
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
        
        // Usage flags
        if (desc->descriptors & CGPU_RESOURCE_TYPE_RENDER_TARGET)
            imageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        else if (is_depth_stencil)
            imageCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if (cubemapRequired)
            imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        if (arrayRequired)
            imageCreateInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;

        SKR_DECLARE_ZERO(VkFormatProperties, format_props);
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
        if (desc->flags & CGPU_TCF_TILED_RESOURCE)
        {
            imageCreateInfo.flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
            imageCreateInfo.flags |= VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
        }

        // Verify that GPU supports this format
        VkFormatFeatureFlags format_features = VkUtil_ImageUsageToFormatFeatures(imageCreateInfo.usage);
        VkFormatFeatureFlags flags = format_props.optimalTilingFeatures & format_features;
        cgpu_assert((flags != 0) && "Format is not supported for GPU local images (i.e. not host visible images)");
        SKR_DECLARE_ZERO(VmaAllocationCreateInfo, mem_reqs)
        if ((desc->flags & CGPU_TCF_ALIASING_RESOURCE) || (desc->flags & CGPU_TCF_TILED_RESOURCE))
        {
            VkResult res = D->mVkDeviceTable.vkCreateImage(D->pVkDevice, &imageCreateInfo, GLOBAL_VkAllocationCallbacks, &pVkImage);
            CHECK_VKRESULT(res);
        }
        else
        {
            // Allocate texture memory
            if (desc->flags & CGPU_TCF_DEDICATED_BIT)
                mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            mem_reqs.usage = (VmaMemoryUsage)VMA_MEMORY_USAGE_GPU_ONLY;

            wchar_t* win32Name = CGPU_NULLPTR;
#if defined(USE_EXTERNAL_MEMORY_EXTENSIONS) && defined(VK_USE_PLATFORM_WIN32_KHR)
            VkExternalMemoryImageCreateInfo externalInfo = { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR, NULL };
            VkExportMemoryAllocateInfo exportMemoryInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR, NULL };
            const wchar_t* nameFormat = L"cgpu-shared-texture-%llu";
            VkExportMemoryWin32HandleInfoKHR win32ExportMemoryInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR, NULL };
            VkImportMemoryWin32HandleInfoKHR win32ImportInfo = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, NULL };
            if (A->external_memory && (desc->flags & CGPU_INNER_TCF_IMPORT_SHARED_HANDLE))
            {
                is_imported = true;
                // format name wstring
                CGPUImportTextureDescriptor* pImportDesc = (CGPUImportTextureDescriptor*)desc->native_handle;
                unique_id = pImportDesc->shared_handle;
                int size_needed = swprintf(CGPU_NULL, 0, nameFormat, unique_id);
                win32Name = cgpu_calloc(1 + size_needed, sizeof(wchar_t));
                swprintf(win32Name, 1 + size_needed, nameFormat, unique_id);
                VkUtil_ImportSharedTexture(Q, &mem_reqs, &imageCreateInfo, desc, win32Name, 
                    &externalInfo, &win32ImportInfo, &pVkImage, &pVkDeviceMemory);
            }
            else if (A->external_memory && desc->flags & CGPU_TCF_EXPORT_BIT)
            {
                // format name wstring
                uint64_t pid = (uint64_t)GetCurrentProcessId();
                uint64_t shared_id = D->next_shared_id++;
                unique_id = (pid << 32) | shared_id;
                int size_needed = swprintf(CGPU_NULL, 0, nameFormat, unique_id);
                win32Name = cgpu_calloc(1 + size_needed, sizeof(wchar_t));
                swprintf(win32Name, 1 + size_needed, nameFormat, unique_id);
                VkUtil_AllocateSharedTexture(Q, &mem_reqs, &imageCreateInfo, desc, win32Name, 
                    &externalInfo, &exportMemoryInfo, &win32ExportMemoryInfo);
            }
#else
            if ((desc->flags & CGPU_TCF_EXPORT_BIT) || (desc->flags & CGPU_INNER_TCF_IMPORT_SHARED_HANDLE))
            {
                cgpu_error("Unsupportted platform detected!");
                return CGPU_NULLPTR;
            }
#endif
            VmaAllocationInfo alloc_info = { 0 };
            if (!is_imported && isSinglePlane)
            {
                if (!desc->is_restrict_dedicated && !is_imported && !(desc->flags & CGPU_TCF_EXPORT_BIT))
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
                SKR_UNIMPLEMENTED_FUNCTION();
            }
#ifdef VK_USE_PLATFORM_WIN32_KHR
            if (win32Name != CGPU_NULLPTR) 
                cgpu_free(win32Name);
#endif
            is_allocation_dedicated = mem_reqs.flags & VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            can_alias_alloc = mem_reqs.flags & VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
        }
    }
    
    // create texture object
    CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)cgpu_calloc_aligned(1, totalSize, _Alignof(CGPUTexture_Vulkan));
    cgpu_assert(T);
    T->pVkImage = pVkImage;
    
    // fill tile info
    CGPUTileTextureSubresourceMapping_Vulkan* pVkTileMappings = CGPU_NULLPTR;
    CGPUTileTexturePackedMipMapping_Vulkan* pVkPackedMappings = CGPU_NULLPTR;
    uint32_t memTypBits = 0;
    if (desc->flags & CGPU_TCF_TILED_RESOURCE)
    {
        VkSparseImageMemoryRequirements sparseReq = VkUtil_FillTiledTextureInfo(D, T, desc, &memTypBits);
        const CGPUTiledTextureInfo* pTiledInfo = T->super.tiled_resource;
        pVkTileMappings = cgpu_calloc_aligned(pTiledInfo->packed_mip_start, sizeof(CGPUTileTextureSubresourceMapping_Vulkan), _Alignof(CGPUTileTextureSubresourceMapping_Vulkan));
        for (uint32_t i = 0; i < pTiledInfo->packed_mip_start; i++)
        {
            const uint32_t X = pTiledInfo->subresources[i].width_in_tiles;
            const uint32_t Y = pTiledInfo->subresources[i].height_in_tiles;
            const uint32_t Z = pTiledInfo->subresources[i].depth_in_tiles;
            CGPUTileTextureSubresourceMapping_Vulkan SM = {
                .X = X,
                .Y = Y,
                .Z = Z,
                .mVkMemoryTypeBits = memTypBits,
                .mappings = cgpu_calloc_aligned(X * Y * Z, sizeof(CGPUTileMapping_Vulkan), _Alignof(CGPUTileMapping_Vulkan))
            };
            memcpy(&pVkTileMappings[i], &SM, sizeof(CGPUTileTextureSubresourceMapping_Vulkan));
        }
        const bool SingleTail = (sparseReq.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT);
        T->mPackedMappingsCount = SingleTail ? 1 : arraySize;
        T->mSingleTail = SingleTail;
        pVkPackedMappings = cgpu_calloc_aligned(T->mPackedMappingsCount, sizeof(CGPUTileTexturePackedMipMapping_Vulkan), _Alignof(CGPUTileTexturePackedMipMapping_Vulkan));
        for (uint32_t i = 0; i < T->mPackedMappingsCount; i++)
        {
            pVkPackedMappings[i].mVkSparseTailStride = sparseReq.imageMipTailStride;
            pVkPackedMappings[i].mVkSparseTailOffset = sparseReq.imageMipTailOffset;
            pVkPackedMappings[i].mVkSparseTailSize = sparseReq.imageMipTailSize;
        }
    }
    // fill texture info
    CGPUTextureInfo* info = (CGPUTextureInfo*)(T + 1);
    T->super.info = info;
    T->pVkTileMappings = pVkTileMappings;
    T->pVkPackedMappings = pVkPackedMappings;
    info->owns_image = owns_image;
    info->aspect_mask = aspect_mask;
    info->is_allocation_dedicated = is_allocation_dedicated;
    info->is_restrict_dedicated = desc->is_restrict_dedicated;
    info->is_aliasing = (desc->flags & CGPU_TCF_ALIASING_RESOURCE);
    info->can_alias = can_alias_alloc || info->is_aliasing;
    if (pVkDeviceMemory) T->pVkDeviceMemory = pVkDeviceMemory;
    if (vmaAllocation) T->pVkAllocation = vmaAllocation;
    info->sample_count = desc->sample_count;
    info->width = desc->width;
    info->height = desc->height;
    info->depth = desc->depth;
    info->mip_levels = desc->mip_levels;
    info->is_cube = cubemapRequired;
    info->array_size_minus_one = arraySize - 1;
    info->format = desc->format;
    info->is_imported = is_imported;
    info->is_tiled = (desc->flags & CGPU_TCF_TILED_RESOURCE) ? 1 : 0;
    info->unique_id = (unique_id == UINT64_MAX) ? D->super.next_texture_id++ : unique_id;
    // Set Texture Name
    VkUtil_OptionalSetObjectName(D, (uint64_t)T->pVkImage, VK_OBJECT_TYPE_IMAGE, desc->name);
    // Start state
    if (Q && T->pVkImage != VK_NULL_HANDLE)
    {
#ifdef CGPU_THREAD_SAFETY
        if (Q->pMutex) skr_mutex_acquire(Q->pMutex);
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
        if (Q->pMutex) skr_mutex_release(Q->pMutex);
#endif
    }
    return &T->super;
}

enum ETileMappingStatus_Vulkan
{
    VK_TILE_MAPPING_STATUS_UNMAPPED = 0,
    VK_TILE_MAPPING_STATUS_PENDING = 1,
    VK_TILE_MAPPING_STATUS_MAPPING = 2,
    VK_TILE_MAPPING_STATUS_MAPPED = 3,
    VK_TILE_MAPPING_STATUS_UNMAPPING = 4
};

CGPUTileMapping_Vulkan* VkUtil_TileMappingAt(CGPUTileTextureSubresourceMapping_Vulkan* subres, uint32_t x, uint32_t y, uint32_t z)
{
    SKR_ASSERT(subres->mappings && x < subres->X && y < subres->Y && z < subres->Z && "SubresTileMappings::at: Out of Range!"); 
    return subres->mappings + (x + y * subres->X + z * subres->X * subres->Y); 
}

CGPUTileTextureSubresourceMapping_Vulkan* VkUtil_GetSubresTileMappings(CGPUTexture_Vulkan* T, uint32_t mip_level, uint32_t array_index)
{
    SKR_ASSERT(mip_level < T->super.info->mip_levels && array_index < T->super.info->array_size_minus_one + 1);
    return T->pVkTileMappings + (mip_level * (T->super.info->array_size_minus_one + 1) + array_index);
}

void VkUtil_UnmapTileMappingAt(CGPUTexture_Vulkan* T, CGPUTileTextureSubresourceMapping_Vulkan* subres, uint32_t x, uint32_t y, uint32_t z)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)T->super.device;
    CGPUTiledTextureInfo* pTiledInfo = (CGPUTiledTextureInfo*)T->super.tiled_resource;
    CGPUTileMapping_Vulkan* Mapping = VkUtil_TileMappingAt(subres, x, y, z);
    int32_t expect_mapped = VK_TILE_MAPPING_STATUS_MAPPED;
    if (skr_atomic_compare_exchange_strong(&Mapping->status, 
            &expect_mapped, VK_TILE_MAPPING_STATUS_UNMAPPING))
    {
        vmaFreeMemory(D->pVmaAllocator, Mapping->pVkAllocation);
        Mapping->pVkAllocation = NULL;
        skr_atomic_fetch_add_relaxed(&pTiledInfo->alive_tiles_count, -1);
    }

    int32_t expect_unmapping = VK_TILE_MAPPING_STATUS_UNMAPPING;
    skr_atomic_compare_exchange_strong(&Mapping->status, 
        &expect_unmapping, VK_TILE_MAPPING_STATUS_UNMAPPED);
}

void VkUtil_UnmapPackedMappingAt(CGPUTexture_Vulkan* T, uint32_t n)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)T->super.device;
    CGPUTiledTextureInfo* pTiledInfo = (CGPUTiledTextureInfo*)T->super.tiled_resource;
    CGPUTileTexturePackedMipMapping_Vulkan* Mapping = &T->pVkPackedMappings[n];
    int32_t expect_mapped = VK_TILE_MAPPING_STATUS_MAPPED;
    if (skr_atomic_compare_exchange_strong(&Mapping->status, 
            &expect_mapped, VK_TILE_MAPPING_STATUS_UNMAPPING))
    {
        vmaFreeMemory(D->pVmaAllocator, Mapping->pVkAllocation);
        Mapping->pVkAllocation = NULL;
        skr_atomic_fetch_add_relaxed(&pTiledInfo->alive_pack_count, -1);
    }    
    int32_t expect_unmapping = VK_TILE_MAPPING_STATUS_UNMAPPING;
    skr_atomic_compare_exchange_strong(&Mapping->status, 
        &expect_unmapping, VK_TILE_MAPPING_STATUS_UNMAPPED);
}

void cgpu_queue_map_packed_mips_vulkan(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)queue->device;
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)queue;
    uint32_t N = 0;
    for (uint32_t i = 0; i < regions->packed_mip_count; i++)
    {
        CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)regions->packed_mips[i].texture;
        uint32_t layer = T->mSingleTail ? 0 : regions->packed_mips[i].layer;
        CGPUTileTexturePackedMipMapping_Vulkan* pMapping = &T->pVkPackedMappings[layer];

        int32_t expect_unmapped = VK_TILE_MAPPING_STATUS_UNMAPPING;
        if (!skr_atomic_compare_exchange_strong(&pMapping->status, 
            &expect_unmapped, VK_TILE_MAPPING_STATUS_PENDING)) continue;

        N++;
    }
    if (N == 0) 
        return;
    
    uint32_t M = 0;
    VkSparseMemoryBind* opaqueBinds = cgpu_calloc(N, sizeof(VkSparseMemoryBind) + sizeof(VkSparseImageOpaqueMemoryBindInfo));
    VkSparseImageOpaqueMemoryBindInfo* bindInfos = (VkSparseImageOpaqueMemoryBindInfo*)(opaqueBinds + N);
    for (uint32_t i = 0; i < regions->packed_mip_count; i++)
    {
        CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)regions->packed_mips[i].texture;
        uint32_t layer = T->mSingleTail ? 0 : regions->packed_mips[i].layer;
        CGPUTileTexturePackedMipMapping_Vulkan* pMapping = &T->pVkPackedMappings[layer];

        int32_t expect_pending = VK_TILE_MAPPING_STATUS_PENDING;
        if (!skr_atomic_compare_exchange_strong(&pMapping->status, &expect_pending, VK_TILE_MAPPING_STATUS_MAPPING)) 
            continue;

        const uint64_t kPageSize = T->pVkPackedMappings->mVkSparseTailSize;
        SKR_DECLARE_ZERO(VkMemoryRequirements, memReqs);
        memReqs.size = kPageSize;
        memReqs.memoryTypeBits = T->pVkTileMappings->mVkMemoryTypeBits;
        memReqs.alignment = memReqs.size;
        SKR_DECLARE_ZERO(VmaAllocationCreateInfo, vmaAllocInfo);
        VmaAllocation pAllocation;
        VmaAllocationInfo AllocationInfo;
        // do allocations
        VkResult result = vmaAllocateMemoryPages(D->pVmaAllocator, &memReqs, 
            &vmaAllocInfo, 1, &pAllocation, &AllocationInfo);
        CHECK_VKRESULT(result);

        pMapping->pVkAllocation = pAllocation;

        opaqueBinds[M].resourceOffset = pMapping->mVkSparseTailOffset + layer * pMapping->mVkSparseTailStride;
		opaqueBinds[M].size = pMapping->mVkSparseTailSize;
		opaqueBinds[M].memory = AllocationInfo.deviceMemory;
		opaqueBinds[M].memoryOffset = AllocationInfo.offset;

        bindInfos[M].pBinds = opaqueBinds + M;
        bindInfos[M].bindCount = 1;
        bindInfos[M].image = T->pVkImage;

        M++;
    }
    SKR_DECLARE_ZERO(VkBindSparseInfo, bindSparseInfo);
    bindSparseInfo.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
    bindSparseInfo.imageOpaqueBindCount = M;
    bindSparseInfo.pImageOpaqueBinds = bindInfos;

    VkResult result = D->mVkDeviceTable.vkQueueBindSparse(Q->pVkQueue, (uint32_t)1, &bindSparseInfo, VK_NULL_HANDLE);
    CHECK_VKRESULT(result);
    
    // mark mapping requests as complete
    for (uint32_t i = 0; i < N; i++)
    {
        CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)regions->packed_mips[i].texture;
        CGPUTiledTextureInfo* pModTiledInfo = (CGPUTiledTextureInfo*)T->super.tiled_resource;
        uint32_t layer = T->mSingleTail ? 0 : regions->packed_mips[i].layer;
        CGPUTileTexturePackedMipMapping_Vulkan* pMapping = &T->pVkPackedMappings[layer];
        
        int32_t expect_mapping = VK_TILE_MAPPING_STATUS_MAPPING;
        skr_atomic_compare_exchange_strong(&pMapping->status, 
            &expect_mapping, VK_TILE_MAPPING_STATUS_MAPPED);
        skr_atomic_fetch_add_relaxed(&pModTiledInfo->alive_pack_count, 1);
    }

    cgpu_free(opaqueBinds);
}

void cgpu_queue_unmap_packed_mips_vulkan(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions)
{
    for (uint32_t i = 0; i < regions->packed_mip_count; i++)
    {
        CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)regions->packed_mips[i].texture;
        uint32_t layer = T->mSingleTail ? 0 : regions->packed_mips[i].layer;

        VkUtil_UnmapPackedMappingAt(T, layer);
    }
}

void cgpu_queue_map_tiled_texture_vulkan(CGPUQueueId queue, const struct CGPUTiledTextureRegions* regions)
{
    const uint32_t kPageSize = VK_SPARSE_PAGE_STANDARD_SIZE;
    const uint32_t RegionCount = regions->region_count;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)queue->device;
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)queue;
    CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)regions->texture;
    const CGPUTiledTextureInfo* pTiledInfo = T->super.tiled_resource;

    // calculate page count
    uint32_t TotalTileCount = 0;
    for (uint32_t i = 0; i < RegionCount; i++)
    {
        const CGPUTextureCoordinateRegion Region = regions->regions[i];
        uint32_t RegionTileCount = 0;
            for (uint32_t x = Region.start.x; x < Region.end.x; x++)
            for (uint32_t y = Region.start.y; y < Region.end.y; y++)
            for (uint32_t z = Region.start.z; z < Region.end.z; z++)
            {
                SKR_ASSERT(Region.mip_level < pTiledInfo->packed_mip_start && 
                    "cgpu_queue_map_tiled_texture_vulkan: Mip level must be less than packed mip start!");
                uint32_t subres_index = Region.layer * T->super.info->mip_levels + Region.mip_level;
                CGPUTileTextureSubresourceMapping_Vulkan* subres = T->pVkTileMappings + subres_index;
                CGPUTileMapping_Vulkan* pMapping = VkUtil_TileMappingAt(subres, x, y, z);

                int32_t expect_unmapped = VK_TILE_MAPPING_STATUS_MAPPING;
                if (skr_atomic_compare_exchange_strong(&pMapping->status, 
                    &expect_unmapped, VK_TILE_MAPPING_STATUS_PENDING))
                {
                    RegionTileCount += 1;
                }
            }
        TotalTileCount += RegionTileCount;
    }
    if (!TotalTileCount) return;

    void* ArgMemory = cgpu_calloc(TotalTileCount, sizeof(VmaAllocation) + sizeof(VmaAllocationInfo) + sizeof(VkSparseImageMemoryBind) + sizeof(CGPUTileMapping_Vulkan*));
    VmaAllocation* pAllocations = (VmaAllocation*)ArgMemory;
    VmaAllocationInfo* pAllocationInfos = (VmaAllocationInfo*)(pAllocations + TotalTileCount);
    VkSparseImageMemoryBind* pBinds = (VkSparseImageMemoryBind*)(pAllocationInfos + TotalTileCount);
    CGPUTileMapping_Vulkan** ppMappings = (CGPUTileMapping_Vulkan**)(pBinds + TotalTileCount);
	SKR_DECLARE_ZERO(VkMemoryRequirements, memReqs);
	memReqs.size = kPageSize;
	memReqs.memoryTypeBits = T->pVkTileMappings->mVkMemoryTypeBits;
	memReqs.alignment = memReqs.size;
	SKR_DECLARE_ZERO(VmaAllocationCreateInfo, vmaAllocInfo);
    // do allocations
    VkResult result = vmaAllocateMemoryPages(D->pVmaAllocator, &memReqs, &vmaAllocInfo, 
        TotalTileCount, pAllocations, pAllocationInfos);
    CHECK_VKRESULT(result);

    // do mapping
    uint32_t AllocateTileCount = 0;
    for (uint32_t i = 0; i < RegionCount; i++)
    {
        const CGPUTextureCoordinateRegion Region = regions->regions[i];
            for (uint32_t x = Region.start.x; x < Region.end.x; x++)
            for (uint32_t y = Region.start.y; y < Region.end.y; y++)
            for (uint32_t z = Region.start.z; z < Region.end.z; z++)
            {
                CGPUTileTextureSubresourceMapping_Vulkan* subres = VkUtil_GetSubresTileMappings(T, Region.mip_level, Region.layer);
                CGPUTileMapping_Vulkan* pMapping = VkUtil_TileMappingAt(subres, x, y, z);
                int32_t expect_pending = VK_TILE_MAPPING_STATUS_PENDING;
                if (!skr_atomic_compare_exchange_strong(&pMapping->status, 
                    &expect_pending, VK_TILE_MAPPING_STATUS_MAPPING)) 
                    continue; // skip if already mapped

                struct VmaAllocation_T* const VkAllocation = pAllocations[AllocateTileCount];
                struct VmaAllocationInfo const VkAllocationInfo = pAllocationInfos[AllocateTileCount];

                VkSparseImageMemoryBind* pBind = pBinds + AllocateTileCount;
                pMapping->pVkAllocation = VkAllocation;
                ppMappings[AllocateTileCount] = pMapping;
                pBind->memory = VkAllocationInfo.deviceMemory;
                pBind->memoryOffset = VkAllocationInfo.offset;

                pBind->subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                pBind->subresource.mipLevel = Region.mip_level;
                pBind->subresource.arrayLayer = Region.layer;

                pBind->offset.x = pTiledInfo->tile_width_in_texels * x;
                pBind->offset.y = pTiledInfo->tile_height_in_texels * y;
                pBind->offset.z = pTiledInfo->tile_depth_in_texels * z;

                pBind->extent.width = pTiledInfo->tile_width_in_texels;
                pBind->extent.height = pTiledInfo->tile_height_in_texels;
                pBind->extent.depth = pTiledInfo->tile_depth_in_texels;

                AllocateTileCount++;
            }
    }

	SKR_DECLARE_ZERO(VkSparseImageMemoryBindInfo, imageMemoryBindInfo);
    imageMemoryBindInfo.image = T->pVkImage;
    imageMemoryBindInfo.bindCount = TotalTileCount;
    imageMemoryBindInfo.pBinds = pBinds;

    SKR_DECLARE_ZERO(VkBindSparseInfo, bindSparseInfo);
    bindSparseInfo.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
    bindSparseInfo.imageBindCount = 1;
    bindSparseInfo.pImageBinds = &imageMemoryBindInfo;

    result = D->mVkDeviceTable.vkQueueBindSparse(Q->pVkQueue, (uint32_t)1, &bindSparseInfo, VK_NULL_HANDLE);
    CHECK_VKRESULT(result);
    
    // mark mapping requests as complete
    CGPUTiledTextureInfo* pModTiledInfo = (CGPUTiledTextureInfo*)T->super.tiled_resource;
    for (uint32_t i = 0; i < AllocateTileCount; i++)
    {
        int32_t expect_mapping = VK_TILE_MAPPING_STATUS_MAPPING;
        skr_atomic_compare_exchange_strong(&ppMappings[i]->status, 
            &expect_mapping, VK_TILE_MAPPING_STATUS_MAPPED);
        skr_atomic_fetch_add_relaxed(&pModTiledInfo->alive_tiles_count, 1);
    }

    cgpu_free(ArgMemory);
}

void cgpu_queue_unmap_tiled_texture_vulkan(CGPUQueueId queue, const struct CGPUTiledTextureRegions* regions)
{
    const uint32_t RegionCount = regions->region_count;
    CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)regions->texture;
    // CGPUTiledTextureInfo* pTiledInfo = (CGPUTiledTextureInfo*)T->super.tiled_resource;
    // CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)queue;
    // CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)Q->super.device;
    // calculate page count
    for (uint32_t i = 0; i < RegionCount; i++)
    {
        const CGPUTextureCoordinateRegion Region = regions->regions[i];
        CGPUTileTextureSubresourceMapping_Vulkan* subres = VkUtil_GetSubresTileMappings(T, Region.mip_level, Region.layer);
        for (uint32_t x = Region.start.x; x < Region.end.x; x++)
        for (uint32_t y = Region.start.y; y < Region.end.y; y++)
        for (uint32_t z = Region.start.z; z < Region.end.z; z++)
        {
            VkUtil_UnmapTileMappingAt(T, subres, x, y, z);

            // CGPUTileMapping_Vulkan* Mapping = VkUtil_TileMappingAt(subres, x, y, z);
            const bool ForceUnmap = false;
            if (ForceUnmap) // slow and only useful for debugging
            {

            }
        }
    }
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
    const CGPUTextureInfo* pInfo = T->super.info;
    if (T->pVkImage != VK_NULL_HANDLE)
    {
        if (pInfo->is_imported)
        {
            D->mVkDeviceTable.vkDestroyImage(D->pVkDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
            D->mVkDeviceTable.vkFreeMemory(D->pVkDevice, T->pVkDeviceMemory, GLOBAL_VkAllocationCallbacks);
        }
        else if (pInfo->owns_image)
        {
            const ECGPUFormat fmt = pInfo->format;
            (void)fmt;
            // TODO: Support planar formats
            const bool isSinglePlane = true;
            if (pInfo->is_tiled)
            {
                D->mVkDeviceTable.vkDestroyImage(D->pVkDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
            }
            else if (isSinglePlane)
            {
                cgpu_trace("Freeing texture allocation %p \n\t size: %dx%dx%d owns_image: %d imported: %d", 
                    T->pVkImage, pInfo->width, pInfo->height, pInfo->depth, pInfo->owns_image, pInfo->is_imported);
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
                    T->pVkImage, pInfo->width, pInfo->height, pInfo->depth, pInfo->owns_image, pInfo->is_imported);

            D->mVkDeviceTable.vkDestroyImage(D->pVkDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
        }
    }
    if (pInfo->is_tiled && T->pVkTileMappings)
    {
        for (uint32_t i = 0; i < T->super.tiled_resource->packed_mip_start; i++)
        {
            CGPUTileTextureSubresourceMapping_Vulkan* subres = T->pVkTileMappings + i;
            if (subres->mappings)
            {
                // unmap & free all mappings
                for (uint32_t x = 0; x < subres->X; x++)
                    for (uint32_t y = 0; y < subres->Y; y++)
                        for (uint32_t z = 0; z < subres->Z; z++)
                            VkUtil_UnmapTileMappingAt(T, subres, x, y, z);
                cgpu_free_aligned(subres->mappings, _Alignof(CGPUTileMapping_Vulkan));
            }
        }
        cgpu_free_aligned(T->pVkTileMappings, _Alignof(CGPUTileTextureSubresourceMapping_Vulkan));

        for (uint32_t n = 0; n < T->mPackedMappingsCount; n++)
            VkUtil_UnmapPackedMappingAt(T, n);
        cgpu_free_aligned(T->pVkPackedMappings, _Alignof(CGPUTileTexturePackedMipMapping_Vulkan));
    }
    if (T->super.tiled_resource)
        cgpu_free_aligned((void*)T->super.tiled_resource, _Alignof(CGPUTiledTextureInfo));
    cgpu_free_aligned(T, _Alignof(CGPUTexture_Vulkan));
}

CGPUTextureViewId cgpu_create_texture_view_vulkan(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)desc->texture->device;
    CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)desc->texture;
    const CGPUTextureInfo* pInfo = T->super.info;
    CGPUTextureView_Vulkan* TV = (CGPUTextureView_Vulkan*)cgpu_calloc_aligned(1, sizeof(CGPUTextureView_Vulkan), _Alignof(CGPUTextureView_Vulkan));
    VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    VkImageType mImageType = pInfo->is_cube ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    switch (mImageType)
    {
        case VK_IMAGE_TYPE_1D:
            view_type = desc->array_layer_count > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;
        case VK_IMAGE_TYPE_2D:
            if (pInfo->is_cube)
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
    cgpu_free_aligned(TV, _Alignof(CGPUTextureView_Vulkan));
}

bool cgpu_try_bind_aliasing_texture_vulkan(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    if (desc->aliased)
    {
        CGPUTexture_Vulkan* Aliased = (CGPUTexture_Vulkan*)desc->aliased;
        CGPUTexture_Vulkan* Aliasing = (CGPUTexture_Vulkan*)desc->aliasing;
        CGPUTextureInfo* AliasingInfo = (CGPUTextureInfo*)Aliasing->super.info;
        const CGPUTextureInfo* AliasedInfo = Aliased->super.info;

        cgpu_assert(AliasingInfo->is_aliasing && "aliasing texture need to be created as aliasing!");
        if (Aliased->pVkImage != VK_NULL_HANDLE && Aliased->pVkAllocation != VK_NULL_HANDLE &&
            Aliasing->pVkImage != VK_NULL_HANDLE &&
            !AliasedInfo->is_restrict_dedicated && AliasingInfo->is_aliasing)
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
    cgpu_free_aligned(S, _Alignof(CGPUSampler_Vulkan));
}

// Shader APIs
CGPUShaderLibraryId cgpu_create_shader_library_vulkan(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = desc->code_size,
        .pCode = desc->code
    };
    CGPUShaderLibrary_Vulkan* S = (CGPUShaderLibrary_Vulkan*)cgpu_calloc(1, sizeof(CGPUShaderLibrary_Vulkan));
    if (!desc->reflection_only)
    {
        D->mVkDeviceTable.vkCreateShaderModule(D->pVkDevice, &info, GLOBAL_VkAllocationCallbacks, &S->mShaderModule);
    }
    VkUtil_InitializeShaderReflection(device, S, desc);
    return &S->super;
}

void cgpu_free_shader_library_vulkan(CGPUShaderLibraryId library)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)library->device;
    CGPUShaderLibrary_Vulkan* S = (CGPUShaderLibrary_Vulkan*)library;
    VkUtil_FreeShaderReflection(S);
    if (S->mShaderModule != VK_NULL_HANDLE)
    {
        D->mVkDeviceTable.vkDestroyShaderModule(D->pVkDevice, S->mShaderModule, GLOBAL_VkAllocationCallbacks);
    }
    cgpu_free(S);
}
