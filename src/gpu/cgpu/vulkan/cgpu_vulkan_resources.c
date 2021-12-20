#include "cgpu/flags.h"
#include "math/common.h"
#include "platform/configure.h"
#include "vulkan_utils.h"
#include <string.h>

FORCEINLINE static VkBufferCreateInfo VkUtil_CreateBufferCreateInfo(CGpuAdapter_Vulkan* A, const struct CGpuBufferDescriptor* desc)
{
    uint64_t allocationSize = desc->size;
    // Align the buffer size to multiples of the dynamic uniform buffer minimum size
    if (desc->descriptors & RT_UNIFORM_BUFFER)
    {
        uint64_t minAlignment = A->adapter_detail.uniform_buffer_alignment;
        allocationSize = smath_round_up_64(allocationSize, minAlignment);
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
    add_info.usage = VkUtil_DescriptorTypesToBufferUsage(desc->descriptors, desc->format != PF_UNDEFINED);
    // Buffer can be used as dest in a transfer command (Uploading data to a storage buffer, Readback query data)
    if (desc->memory_usage == MU_GPU_ONLY || desc->memory_usage == MU_GPU_TO_CPU)
        add_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return add_info;
}

FORCEINLINE static VkImageAspectFlags VkUtil_DeterminAspectMask(VkFormat format, bool includeStencilBit)
{
    VkImageAspectFlags result = 0;
    switch (format)
    {
        // Depth
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
        // Stencil
        case VK_FORMAT_S8_UINT:
            result = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        // Depth/stencil
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (includeStencilBit)
                result |= VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        // Assume everything else is Color
        default:
            result = VK_IMAGE_ASPECT_COLOR_BIT;
            break;
    }
    return result;
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

// Buffer APIs
static_assert(sizeof(CGpuBuffer_Vulkan) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line
CGpuBufferId cgpu_create_buffer_vulkan(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc)
{
    CGpuBuffer_Vulkan* B = cgpu_calloc_aligned(1, sizeof(CGpuBuffer_Vulkan), _Alignof(CGpuBuffer_Vulkan));
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;
    // Create VkBufferCreateInfo
    VkBufferCreateInfo add_info = VkUtil_CreateBufferCreateInfo(A, desc);
    // VMA Alloc
    VmaAllocationCreateInfo vma_mem_reqs = { .usage = (VmaMemoryUsage)desc->memory_usage };
    if (desc->flags & BCF_OWN_MEMORY_BIT)
        vma_mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    if (desc->flags & BCF_PERSISTENT_MAP_BIT)
        vma_mem_reqs.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

    DECLARE_ZERO(VmaAllocationInfo, alloc_info)
    VkResult bufferResult =
        vmaCreateBuffer(D->pVmaAllocator, &add_info, &vma_mem_reqs, &B->pVkBuffer, &B->pVkAllocation, &alloc_info);
    if (bufferResult != VK_SUCCESS)
    {
        assert(0);
        return NULL;
    }
    B->super.cpu_mapped_address = alloc_info.pMappedData;

    // Setup Descriptors
    if ((desc->descriptors & RT_UNIFORM_BUFFER) || (desc->descriptors & RT_BUFFER) ||
        (desc->descriptors & RT_RW_BUFFER))
    {
        if ((desc->descriptors & RT_BUFFER) || (desc->descriptors & RT_RW_BUFFER))
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
    B->super.size = (uint32_t)desc->size;
    B->super.memory_usage = desc->memory_usage;
    B->super.descriptors = desc->descriptors;
    return &B->super;
}

void cgpu_map_buffer_vulkan(CGpuBufferId buffer, const struct CGpuBufferRange* range)
{
    CGpuBuffer_Vulkan* B = (CGpuBuffer_Vulkan*)buffer;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)B->super.device;
    assert(B->super.memory_usage != MU_GPU_ONLY && "Trying to map non-cpu accessible resource");

    VkResult vk_res = vmaMapMemory(D->pVmaAllocator, B->pVkAllocation, &B->super.cpu_mapped_address);
    assert(vk_res == VK_SUCCESS);

    if (range && (vk_res == VK_SUCCESS))
    {
        B->super.cpu_mapped_address = ((uint8_t*)B->super.cpu_mapped_address + range->offset);
    }
}

void cgpu_unmap_buffer_vulkan(CGpuBufferId buffer)
{
    CGpuBuffer_Vulkan* B = (CGpuBuffer_Vulkan*)buffer;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)B->super.device;
    assert(B->super.memory_usage != MU_GPU_ONLY && "Trying to unmap non-cpu accessible resource");

    vmaUnmapMemory(D->pVmaAllocator, B->pVkAllocation);
    B->super.cpu_mapped_address = CGPU_NULLPTR;
}

void cgpu_cmd_update_buffer_vulkan(CGpuCommandBufferId cmd, const struct CGpuBufferUpdateDescriptor* desc)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cmd;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cmd->device;
    CGpuBuffer_Vulkan* Dst = (CGpuBuffer_Vulkan*)desc->dst;
    CGpuBuffer_Vulkan* Src = (CGpuBuffer_Vulkan*)desc->src;
    VkBufferCopy region = {
        .srcOffset = desc->src_offset,
        .dstOffset = desc->dst_offset,
        .size = desc->size
    };
    D->mVkDeviceTable.vkCmdCopyBuffer(Cmd->pVkCmdBuf, Src->pVkBuffer, Dst->pVkBuffer, 1, &region);
}

void cgpu_free_buffer_vulkan(CGpuBufferId buffer)
{
    CGpuBuffer_Vulkan* B = (CGpuBuffer_Vulkan*)buffer;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)B->super.device;
    assert(B->pVkAllocation && "pVkAllocation must not be null!");
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
    cgpu_free(B);
}

// Texture/RenderTarget APIs
static_assert(sizeof(CGpuTexture_Vulkan) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line
typedef struct ImportHandleInfo {
    void* pHandle;
    VkExternalMemoryHandleTypeFlagBitsKHR mHandleType;
} ImportHandleInfo;
CGpuTextureId cgpu_create_texture_vulkan(CGpuDeviceId device, const struct CGpuTextureDescriptor* desc)
{
    if (desc->sample_count > SC_1 && desc->mip_levels > 1)
    {
        cgpu_error("Multi-Sampled textures cannot have mip maps");
        assert(false);
        return CGPU_NULLPTR;
    }
    // Alloc aligned memory
    size_t totalSize = sizeof(CGpuTexture_Vulkan);
    totalSize += (desc->descriptors & RT_RW_TEXTURE ? (desc->mip_levels * sizeof(VkImageView)) : 0);
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;
    CGpuTexture_Vulkan* T = (CGpuTexture_Vulkan*)cgpu_calloc_aligned(1, totalSize, _Alignof(CGpuTexture_Vulkan));
    const CGpuFormatSupport* format_support = &A->adapter_detail.format_supports[desc->format];
    assert(T);
    //
    if (desc->descriptors & RT_RW_TEXTURE)
        T->pVkUAVDescriptors = (VkImageView*)(T + 1);
    if (desc->native_handle && !(desc->flags & TCF_IMPORT_BIT))
    {
        T->super.owns_image = false;
        T->pVkImage = (VkImage)desc->native_handle;
    }
    else
        T->super.owns_image = true;
    // Usage flags
    VkImageUsageFlags additionalFlags = 0;
    if (desc->start_state & RS_RENDER_TARGET)
        additionalFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    else if (desc->start_state & RS_DEPTH_WRITE)
        additionalFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    uint32_t arraySize = desc->array_size;
    // Image type
    VkImageType image_type = VK_IMAGE_TYPE_MAX_ENUM;
    if (desc->flags & TCF_FORCE_2D)
    {
        assert(desc->depth == 1);
        image_type = VK_IMAGE_TYPE_2D;
    }
    else if (desc->flags & TCF_FORCE_3D)
    {
        image_type = VK_IMAGE_TYPE_3D;
    }
    else
    {
        if (desc->depth > 1)
            image_type = VK_IMAGE_TYPE_3D;
        else if (desc->height > 1)
            image_type = VK_IMAGE_TYPE_2D;
        else
            image_type = VK_IMAGE_TYPE_1D;
    }

    CGpuResourceTypes descriptors = desc->descriptors;
    bool cubemapRequired = (RT_TEXTURE_CUBE == (descriptors & RT_TEXTURE_CUBE));
    bool arrayRequired = image_type == VK_IMAGE_TYPE_3D;
    // TODO: Support stencil format
    const bool isTencilFormat = false;
    // TODO: Support planar format
    const bool isPlanarFormat = false;
    const uint32_t numOfPlanes = 1;
    const bool isSinglePlane = true;
    assert(
        ((isSinglePlane && numOfPlanes == 1) || (!isSinglePlane && numOfPlanes > 1 && numOfPlanes <= MAX_PLANE_COUNT)) &&
        "Number of planes for multi-planar formats must be 2 or 3 and for single-planar formats it must be 1.");

    if (VK_NULL_HANDLE == T->pVkImage)
    {
        VkImageCreateInfo add_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .imageType = image_type,
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
        add_info.usage |= additionalFlags;
        if (cubemapRequired)
            add_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        if (arrayRequired)
            add_info.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;

        DECLARE_ZERO(VkFormatProperties, format_props);
        vkGetPhysicalDeviceFormatProperties(A->pPhysicalDevice, add_info.format, &format_props);
        if (isPlanarFormat) // multi-planar formats must have each plane separately bound to memory, rather than having a single memory binding for the whole image
        {
            assert(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT);
            add_info.flags |= VK_IMAGE_CREATE_DISJOINT_BIT;
        }

        if ((VK_IMAGE_USAGE_SAMPLED_BIT & add_info.usage) || (VK_IMAGE_USAGE_STORAGE_BIT & add_info.usage))
        {
            // Make it easy to copy to and from textures
            add_info.usage |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        }
        assert(format_support->shader_read && "GPU shader can't' read from this format");
        // Verify that GPU supports this format
        VkFormatFeatureFlags format_features = VkUtil_ImageUsageToFormatFeatures(add_info.usage);
        VkFormatFeatureFlags flags = format_props.optimalTilingFeatures & format_features;
        assert((0 != flags) && "Format is not supported for GPU local images (i.e. not host visible images)");
        // Allocate texture memory
        VmaAllocationCreateInfo mem_reqs = { 0 };
        if (desc->flags & TCF_OWN_MEMORY_BIT)
            mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        mem_reqs.usage = (VmaMemoryUsage)VMA_MEMORY_USAGE_GPU_ONLY;
        VkExternalMemoryImageCreateInfoKHR externalInfo = { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR, NULL };
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VkImportMemoryWin32HandleInfoKHR importInfo = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, NULL };
#endif
        VkExportMemoryAllocateInfoKHR exportMemoryInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR, NULL };
        if (A->external_memory && desc->flags & TCF_IMPORT_BIT)
        {
            add_info.pNext = &externalInfo;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            ImportHandleInfo* pHandleInfo = (ImportHandleInfo*)desc->native_handle;
            importInfo.handle = pHandleInfo->pHandle;
            importInfo.handleType = pHandleInfo->mHandleType;

            externalInfo.handleTypes = pHandleInfo->mHandleType;

            mem_reqs.pUserData = &importInfo;
            // Allocate external (importable / exportable) memory as dedicated memory to avoid adding unnecessary complexity to the Vulkan Memory Allocator
            mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
#endif
        }
        else if (A->external_memory && desc->flags & TCF_EXPORT_BIT)
        {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            exportMemoryInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#endif
            mem_reqs.pUserData = &exportMemoryInfo;
            // Allocate external (importable / exportable) memory as dedicated memory to avoid adding unnecessary complexity to the Vulkan Memory Allocator
            mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        }
        VmaAllocationInfo alloc_info = { 0 };
        if (isSinglePlane)
        {
            VkResult res = vmaCreateImage(D->pVmaAllocator,
                &add_info, &mem_reqs, &T->pVkImage, &T->pVkAllocation, &alloc_info);
            CHECK_VKRESULT(res);
        }
        else // Multi-planar formats
        {
            // TODO: Planar formats
        }
    }
    /************************************************************************/
    // Create image view
    /************************************************************************/
    VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    switch (image_type)
    {
        case VK_IMAGE_TYPE_1D:
            view_type = arraySize > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;
        case VK_IMAGE_TYPE_2D:
            if (cubemapRequired)
                view_type = (arraySize > 6) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
            else
                view_type = arraySize > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            break;
        case VK_IMAGE_TYPE_3D:
            if (arraySize > 1)
            {
                cgpu_error("Cannot support 3D Texture Array in Vulkan");
                assert(false);
            }
            view_type = VK_IMAGE_VIEW_TYPE_3D;
            break;
        default:
            assert(false && "Image Format not supported!");
            break;
    }
    assert(view_type != VK_IMAGE_VIEW_TYPE_MAX_ENUM && "Invalid Image View");
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
        .subresourceRange.aspectMask = VkUtil_DeterminAspectMask(srvDesc.format, false),
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = desc->mip_levels,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = arraySize
    };
    T->super.aspect_mask = VkUtil_DeterminAspectMask(srvDesc.format, true);
    if (descriptors & RT_TEXTURE)
    {
        CHECK_VKRESULT(D->mVkDeviceTable.vkCreateImageView(D->pVkDevice,
            &srvDesc, GLOBAL_VkAllocationCallbacks, &T->pVkSRVDescriptor));
    }
    // SRV stencil
    if (isTencilFormat && (descriptors & RT_TEXTURE))
    {
        srvDesc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        CHECK_VKRESULT(D->mVkDeviceTable.vkCreateImageView(D->pVkDevice,
            &srvDesc, GLOBAL_VkAllocationCallbacks, &T->pVkSRVStencilDescriptor));
    }
    // UAV
    if (descriptors & RT_RW_TEXTURE)
    {
        VkImageViewCreateInfo uavDesc = srvDesc;
        // #NOTE : We dont support imageCube, imageCubeArray for consistency with other APIs
        // All cubemaps will be used as image2DArray for Image Load / Store ops
        if (uavDesc.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY || uavDesc.viewType == VK_IMAGE_VIEW_TYPE_CUBE)
            uavDesc.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        uavDesc.subresourceRange.levelCount = 1;
        for (uint32_t i = 0; i < desc->mip_levels; ++i)
        {
            uavDesc.subresourceRange.baseMipLevel = i;
            CHECK_VKRESULT(D->mVkDeviceTable.vkCreateImageView(D->pVkDevice,
                &uavDesc, GLOBAL_VkAllocationCallbacks, &T->pVkUAVDescriptors[i]));
        }
    }
    /************************************************************************/
    /************************************************************************/
    T->super.width = desc->width;
    T->super.height = desc->height;
    T->super.depth = desc->depth;
    T->super.mip_levels = desc->mip_levels;
    T->super.uav = desc->descriptors & RT_RW_TEXTURE;
    T->super.array_size_minus_one = arraySize - 1;
    T->super.format = desc->format;
    // Set Texture Name
    VkUtil_OptionalSetObjectName(D, (uint64_t)T->pVkImage, VK_OBJECT_TYPE_IMAGE, desc->name);
    return &T->super;
}

void cgpu_free_texture_vulkan(CGpuTextureId texture)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)texture->device;
    CGpuTexture_Vulkan* T = (CGpuTexture_Vulkan*)texture;
    if (T->super.owns_image)
    {
        const ECGpuFormat fmt = texture->format;
        (void)fmt;
        // TODO: Support planar formats
        const bool isSinglePlane = true;
        if (isSinglePlane)
        {
            vmaDestroyImage(D->pVmaAllocator, T->pVkImage, T->pVkAllocation);
        }
        else
        {
            D->mVkDeviceTable.vkDestroyImage(D->pVkDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
            D->mVkDeviceTable.vkFreeMemory(D->pVkDevice, T->pVkDeviceMemory, GLOBAL_VkAllocationCallbacks);
        }
    }
    // Free descriptors
    if (VK_NULL_HANDLE != T->pVkSRVDescriptor)
        D->mVkDeviceTable.vkDestroyImageView(D->pVkDevice, T->pVkSRVDescriptor, GLOBAL_VkAllocationCallbacks);
    if (VK_NULL_HANDLE != T->pVkSRVStencilDescriptor)
        D->mVkDeviceTable.vkDestroyImageView(D->pVkDevice, T->pVkSRVStencilDescriptor, GLOBAL_VkAllocationCallbacks);
    if (T->pVkUAVDescriptors)
    {
        for (uint32_t i = 0; i < T->super.mip_levels; ++i)
        {
            D->mVkDeviceTable.vkDestroyImageView(D->pVkDevice, T->pVkUAVDescriptors[i], GLOBAL_VkAllocationCallbacks);
        }
    }
    cgpu_free(T);
}

CGpuRenderTargetId cgpu_create_render_target_vulkan(CGpuDeviceId device, const struct CGpuRenderTargetDescriptor* desc)
{
    // TODO: Support Depth Format
    bool isDepth = false;
    (void)isDepth;
    uint32_t arraySize = desc->array_size;
    uint32_t depthOrArraySize = arraySize * desc->depth;
    uint32_t numRTVs = desc->mip_levels;
    if ((desc->descriptors & RT_RENDER_TARGET_ARRAY_SLICES) ||
        (desc->descriptors & RT_RENDER_TARGET_DEPTH_SLICES))
    {
        numRTVs *= depthOrArraySize;
    }
    size_t totalSize = sizeof(CGpuRenderTarget_Vulkan);
    totalSize += numRTVs * sizeof(VkImageView);
    CGpuRenderTarget_Vulkan* pRenderTarget = (CGpuRenderTarget_Vulkan*)
        cgpu_calloc_aligned(1, totalSize, _Alignof(CGpuRenderTarget_Vulkan));

    return &pRenderTarget->super;
}

void cgpu_free_render_target_vulkan(CGpuRenderTargetId render_target)
{
    CGpuRenderTarget_Vulkan* RT = (CGpuRenderTarget_Vulkan*)render_target;
    cgpu_free(RT);
}

// Shader APIs
CGpuShaderLibraryId cgpu_create_shader_library_vulkan(
    CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = desc->code_size,
        .pCode = desc->code
    };
    CGpuShaderLibrary_Vulkan* S = (CGpuShaderLibrary_Vulkan*)cgpu_calloc(1, sizeof(CGpuShaderLibrary_Vulkan));
    D->mVkDeviceTable.vkCreateShaderModule(D->pVkDevice, &info, GLOBAL_VkAllocationCallbacks, &S->mShaderModule);
    VkUtil_InitializeShaderReflection(device, S, desc);
    return &S->super;
}

void cgpu_free_shader_library_vulkan(CGpuShaderLibraryId library)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)library->device;
    CGpuShaderLibrary_Vulkan* S = (CGpuShaderLibrary_Vulkan*)library;
    VkUtil_FreeShaderReflection(S);
    D->mVkDeviceTable.vkDestroyShaderModule(D->pVkDevice, S->mShaderModule, GLOBAL_VkAllocationCallbacks);
    cgpu_free(S);
}