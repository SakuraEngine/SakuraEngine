#define DLL_IMPLEMENTATION
#include "vulkan_utils.h"
#include "math/common.h"
#include <assert.h>
#include <string.h>
#ifdef _WINDOWS
    #include <malloc.h>
#endif

const CGpuProcTable tbl_vk = {
    //
    .create_instance = &cgpu_create_instance_vulkan,
    .query_instance_features = &cgpu_query_instance_features_vulkan,
    .free_instance = &cgpu_free_instance_vulkan,

    .enum_adapters = &cgpu_enum_adapters_vulkan,
    .query_adapter_detail = &cgpu_query_adapter_detail_vulkan,
    .query_queue_count = &cgpu_query_queue_count_vulkan,

    .create_device = &cgpu_create_device_vulkan,
    .free_device = &cgpu_free_device_vulkan,

    .get_queue = &cgpu_get_queue_vulkan,
    .free_queue = &cgpu_free_queue_vulkan,

    .create_command_pool = &cgpu_create_command_pool_vulkan,
    .free_command_pool = &cgpu_free_command_pool_vulkan,

    .create_buffer = &cgpu_create_buffer_vulkan,
    .free_buffer = &cgpu_free_buffer_vulkan,

    .create_shader_library = &cgpu_create_shader_library_vulkan,
    .free_shader_library = &cgpu_free_shader_library_vulkan,

    .create_swapchain = &cgpu_create_swapchain_vulkan,
    .free_swapchain = &cgpu_free_swapchain_vulkan
    //
};

const CGpuProcTable* CGPU_VulkanProcTable() { return &tbl_vk; }

void cgpu_query_instance_features_vulkan(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
    features->specialization_constant = true;
}

void cgpu_enum_adapters_vulkan(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)instance;
    *adapters_num = I->mPhysicalDeviceCount;
    if (adapters != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++)
        {
            adapters[i] = &I->pVulkanAdapters[i].super;
        }
    }
}

void cgpu_query_adapter_detail_vulkan(const CGpuAdapterId adapter, struct CGpuAdapterDetail* detail)
{
    CGpuAdapter_Vulkan* a = (CGpuAdapter_Vulkan*)adapter;
    detail->deviceId = a->mPhysicalDeviceProps.properties.deviceID;
    detail->vendorId = a->mPhysicalDeviceProps.properties.vendorID;
    detail->name = a->mPhysicalDeviceProps.properties.deviceName;

    detail->uniform_buffer_alignment = a->mPhysicalDeviceProps.properties.limits.minUniformBufferOffsetAlignment;
    detail->upload_buffer_texture_alignment =
        a->mPhysicalDeviceProps.properties.limits.optimalBufferCopyOffsetAlignment;
    detail->upload_buffer_texture_row_alignment =
        a->mPhysicalDeviceProps.properties.limits.optimalBufferCopyRowPitchAlignment;
    detail->max_vertex_input_bindings = a->mPhysicalDeviceProps.properties.limits.maxVertexInputBindings;
    detail->multidraw_indirect = a->mPhysicalDeviceProps.properties.limits.maxDrawIndirectCount > 1;
    detail->wave_lane_count = a->mSubgroupProperties.subgroupSize;
}

uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    CGpuAdapter_Vulkan* a = (CGpuAdapter_Vulkan*)adapter;
    uint32_t count = 0;
    switch (type)
    {
        case ECGpuQueueType_Graphics: {
            for (uint32_t i = 0; i < a->mQueueFamilyPropertiesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &a->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    count += prop->queueCount;
                }
            }
        }
        break;
        case ECGpuQueueType_Compute: {
            for (uint32_t i = 0; i < a->mQueueFamilyPropertiesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &a->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    if (!(prop->queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    {
                        count += prop->queueCount;
                    }
                }
            }
        }
        break;
        case ECGpuQueueType_Transfer: {
            for (uint32_t i = 0; i < a->mQueueFamilyPropertiesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &a->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    if (!(prop->queueFlags & VK_QUEUE_COMPUTE_BIT))
                    {
                        if (!(prop->queueFlags & VK_QUEUE_GRAPHICS_BIT))
                        {
                            count += prop->queueCount;
                        }
                    }
                }
            }
        }
        break;
        default:
            assert(0 && "CGPU VULKAN: ERROR Queue Type!");
    }
    return count;
}

CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    assert(device && "CGPU VULKAN: NULL DEVICE!");
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;

    CGpuQueue_Vulkan Q = {.super = {.device = &D->super, .index = index, .type = type}};
    D->mVkDeviceTable.vkGetDeviceQueue(D->pVkDevice, (uint32_t)A->mQueueFamilyIndices[type], index, &Q.pVkQueue);
    Q.mVkQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[type];

    CGpuQueue_Vulkan* RQ = (CGpuQueue_Vulkan*)cgpu_calloc(1, sizeof(CGpuQueue_Vulkan));
    memcpy(RQ, &Q, sizeof(Q));
    return &RQ->super;
}

void cgpu_free_queue_vulkan(CGpuQueueId queue) { cgpu_free((void*)queue); }

VkCommandPool allocate_transient_command_pool(CGpuDevice_Vulkan* D, CGpuQueueId queue)
{
    VkCommandPool P = VK_NULL_HANDLE;
    // CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)queue;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)queue->device->adapter;

    VkCommandPoolCreateInfo create_info = {
        //
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        // transient.
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[queue->type]
        //
    };

    if (VK_SUCCESS !=
#ifdef VK_USE_VOLK_DEVICE_TABLE
        D->mVkDeviceTable.
#endif
        vkCreateCommandPool(D->pVkDevice, &create_info, GLOBAL_VkAllocationCallbacks, &P))
    {
        assert(0 && "CGPU VULKAN: CREATE COMMAND POOL FAILED!");
    }
    return P;
}

void free_transient_command_pool(CGpuDevice_Vulkan* D, VkCommandPool pool)
{
#ifdef VK_USE_VOLK_DEVICE_TABLE
    D->mVkDeviceTable.
#endif
        vkDestroyCommandPool(D->pVkDevice, pool, GLOBAL_VkAllocationCallbacks);
}

CGpuCommandPoolId cgpu_create_command_pool_vulkan(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)queue->device;
    CGpuCommandPool_Vulkan* E = (CGpuCommandPool_Vulkan*)cgpu_calloc(1, sizeof(CGpuCommandPool_Vulkan));
    E->pVkCmdPool = allocate_transient_command_pool(D, queue);
    return &E->super;
}

void cgpu_free_command_pool_vulkan(CGpuCommandPoolId encoder)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)encoder->queue->device;
    CGpuCommandPool_Vulkan* E = (CGpuCommandPool_Vulkan*)encoder;
    free_transient_command_pool(D, E->pVkCmdPool);
    cgpu_free((void*)encoder);
}

#define clamp(x, min, max) (x) < (min) ? (min) : ((x) > (max) ? (max) : (x));

// Shader APIs
CGpuShaderLibraryId cgpu_create_shader_library_vulkan(
    CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    VkShaderModuleCreateInfo info = {
        //
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = desc->code_size,
        .pCode = desc->code
        //
    };
    CGpuShaderLibrary_Vulkan* S = (CGpuShaderLibrary_Vulkan*)cgpu_calloc(1, sizeof(CGpuSwapChain_Vulkan));
    D->mVkDeviceTable.vkCreateShaderModule(D->pVkDevice, &info, GLOBAL_VkAllocationCallbacks, &S->mShaderModule);
    return &S->super;
}

void cgpu_free_shader_library_vulkan(CGpuShaderLibraryId module)
{
    CGpuShaderLibrary_Vulkan* S = (CGpuShaderLibrary_Vulkan*)module;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)module->device;
    D->mVkDeviceTable.vkDestroyShaderModule(D->pVkDevice, S->mShaderModule, GLOBAL_VkAllocationCallbacks);
    cgpu_free(S);
}

// Buffer APIs
CGpuBufferId cgpu_create_buffer_vulkan(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc)
{
    CGpuBuffer_Vulkan* B = cgpu_calloc_aligned(1, sizeof(CGpuBuffer_Vulkan), _Alignof(CGpuBuffer_Vulkan));
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;
    uint64_t allocationSize = desc->size;
    // Align the buffer size to multiples of the dynamic uniform buffer minimum size
    if (desc->descriptors & DT_UNIFORM_BUFFER)
    {
        uint64_t minAlignment = A->mPhysicalDeviceProps.properties.limits.minUniformBufferOffsetAlignment;
        allocationSize = smath_round_up_64(allocationSize, minAlignment);
    }
    VkBufferCreateInfo add_info = {
        //
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = allocationSize,
        // Queues Props
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL
        //
    };
    add_info.usage = VkUtil_DescriptorTypesToBufferUsage(desc->descriptors, desc->format != PF_UNDEFINED);
    // Buffer can be used as dest in a transfer command (Uploading data to a storage buffer, Readback query data)
    if (desc->memory_usage == MU_GPU_ONLY || desc->memory_usage == MU_GPU_TO_CPU)
        add_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo vma_mem_reqs = {.usage = (VmaMemoryUsage)desc->memory_usage};
    // if (desc->mFlags & BUFFER_CREATION_FLAG_OWN_MEMORY_BIT)
    //	vma_mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    // if (desc->mFlags & BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT)
    //	vma_mem_reqs.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

    DECLARE_ZERO(VmaAllocationInfo, alloc_info)
    VkResult bufferResult =
        vmaCreateBuffer(D->pVmaAllocator, &add_info, &vma_mem_reqs, &B->pVkBuffer, &B->pVkAllocation, &alloc_info);
    if (bufferResult != VK_SUCCESS)
    {
        assert(0);
        return NULL;
    }
    B->super.cpu_mapped_address = alloc_info.pMappedData;

    return &B->super;
}

void cgpu_free_buffer_vulkan(CGpuBufferId buffer)
{
    CGpuBuffer_Vulkan* B = (CGpuBuffer_Vulkan*)buffer;

    cgpu_free(B);
}

// SwapChain APIs
CGpuSwapChainId cgpu_create_swapchain_vulkan(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;
    // CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)device->adapter->instance;
    VkSurfaceKHR vkSurface = (VkSurfaceKHR)desc->surface;

    VkSurfaceCapabilitiesKHR caps = {0};
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(A->pPhysicalDevice, vkSurface, &caps) != VK_SUCCESS)
    {
        assert(0 && "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!");
    }
    if ((caps.maxImageCount > 0) && (desc->imageCount > caps.maxImageCount))
    {
        ((CGpuSwapChainDescriptor*)desc)->imageCount = caps.maxImageCount;
    }
    else if (desc->imageCount < caps.minImageCount)
    {
        ((CGpuSwapChainDescriptor*)desc)->imageCount = caps.minImageCount;
    }

    // Surface format
    // Select a surface format, depending on whether HDR is available.
    DECLARE_ZERO(VkSurfaceFormatKHR, surface_format)
    surface_format.format = VK_FORMAT_UNDEFINED;
    uint32_t surfaceFormatCount = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(A->pPhysicalDevice,
            vkSurface, &surfaceFormatCount, CGPU_NULLPTR) != VK_SUCCESS)
    {
        assert(0 && "fatal: vkGetPhysicalDeviceSurfaceFormatsKHR failed!");
    }
    // Allocate and get surface formats
    VkSurfaceFormatKHR* formats = NULL;
    formats = (VkSurfaceFormatKHR*)cgpu_calloc(surfaceFormatCount, sizeof(VkSurfaceFormatKHR));
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(A->pPhysicalDevice,
            vkSurface, &surfaceFormatCount, formats) != VK_SUCCESS)
    {
        assert(0 && "fatal: vkGetPhysicalDeviceSurfaceFormatsKHR failed!");
    }

    const VkSurfaceFormatKHR hdrSurfaceFormat = {
        //
        VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        VK_COLOR_SPACE_HDR10_ST2084_EXT
        //
    };
    if ((1 == surfaceFormatCount) && (VK_FORMAT_UNDEFINED == formats[0].format))
    {
        surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        VkFormat requested_format = pf_translate_to_vulkan(desc->format);
        VkColorSpaceKHR requested_color_space = requested_format == hdrSurfaceFormat.format ? hdrSurfaceFormat.colorSpace : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        for (uint32_t i = 0; i < surfaceFormatCount; ++i)
        {
            if ((requested_format == formats[i].format) && (requested_color_space == formats[i].colorSpace))
            {
                surface_format.format = requested_format;
                surface_format.colorSpace = requested_color_space;
                break;
            }
        }
        // Default to VK_FORMAT_B8G8R8A8_UNORM if requested format isn't found
        if (VK_FORMAT_UNDEFINED == surface_format.format)
        {
            surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
            surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
    }
    assert(VK_FORMAT_UNDEFINED != surface_format.format);
    cgpu_free(formats);

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t swapChainImageCount = 0;
    VkPresentModeKHR* modes = NULL;
    // Get present mode count
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(A->pPhysicalDevice, vkSurface, &swapChainImageCount, NULL))
    {
        assert(0 && "fatal: vkGetPhysicalDeviceSurfacePresentModesKHR failed!");
    }

    // Allocate and get present modes
    modes = (VkPresentModeKHR*)alloca(swapChainImageCount * sizeof(*modes));
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(A->pPhysicalDevice, vkSurface, &swapChainImageCount, modes))
    {
        assert(0 && "fatal: vkGetPhysicalDeviceSurfacePresentModesKHR failed!");
    }
    VkPresentModeKHR preferredModeList[] = {
        VK_PRESENT_MODE_IMMEDIATE_KHR,    // normal
        VK_PRESENT_MODE_MAILBOX_KHR,      // low latency
        VK_PRESENT_MODE_FIFO_RELAXED_KHR, // minimize stuttering
        VK_PRESENT_MODE_FIFO_KHR          // low power consumption
    };
    const uint32_t preferredModeCount = CGPU_ARRAY_LEN(preferredModeList);

    uint32_t preferredModeStartIndex = desc->enableVsync ? 1 : 0;
    for (uint32_t j = preferredModeStartIndex; j < preferredModeCount; ++j)
    {
        VkPresentModeKHR mode = preferredModeList[j];
        uint32_t i = 0;
        for (i = 0; i < swapChainImageCount; ++i)
        {
            if (modes[i] == mode)
            {
                break;
            }
        }
        if (i < swapChainImageCount)
        {
            present_mode = mode;
            break;
        }
    }

    // Swapchain
    VkExtent2D extent;
    extent.width = clamp(desc->width, caps.minImageExtent.width, caps.maxImageExtent.width);
    extent.height = clamp(desc->height, caps.minImageExtent.height, caps.maxImageExtent.height);

    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)desc->presentQueues[0];
    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t presentQueueFamilyIndex = -1;
    // Check Queue Present Support.
    {
        VkBool32 sup = VK_FALSE;
        VkResult res =
            vkGetPhysicalDeviceSurfaceSupportKHR(A->pPhysicalDevice, Q->mVkQueueFamilyIndex, vkSurface, &sup);
        if ((VK_SUCCESS == res) && (VK_TRUE == sup))
        {
            presentQueueFamilyIndex = Q->mVkQueueFamilyIndex;
        }
        else
        {
            // Get queue family properties
            uint32_t queueFamilyPropertyCount = 0;
            VkQueueFamilyProperties* queueFamilyProperties = NULL;
            vkGetPhysicalDeviceQueueFamilyProperties(A->pPhysicalDevice, &queueFamilyPropertyCount, NULL);
            queueFamilyProperties =
                (VkQueueFamilyProperties*)alloca(queueFamilyPropertyCount * sizeof(VkQueueFamilyProperties));
            vkGetPhysicalDeviceQueueFamilyProperties(
                A->pPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties);

            // Check if hardware provides dedicated present queue
            if (queueFamilyPropertyCount)
            {
                for (uint32_t index = 0; index < queueFamilyPropertyCount; ++index)
                {
                    VkBool32 supports_present = VK_FALSE;
                    VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(
                        A->pPhysicalDevice, index, vkSurface, &supports_present);
                    if ((VK_SUCCESS == res) && (VK_TRUE == supports_present) && Q->mVkQueueFamilyIndex != index)
                    {
                        presentQueueFamilyIndex = index;
                        break;
                    }
                }
                // If there is no dedicated present queue, just find the first available queue which supports
                // present
                if (presentQueueFamilyIndex == -1)
                {
                    for (uint32_t index = 0; index < queueFamilyPropertyCount; ++index)
                    {
                        VkBool32 supports_present = VK_FALSE;
                        VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(
                            A->pPhysicalDevice, index, vkSurface, &supports_present);
                        if ((VK_SUCCESS == res) && (VK_TRUE == supports_present))
                        {
                            presentQueueFamilyIndex = index;
                            break;
                        }
                        else
                        {
                            // No present queue family available. Something goes wrong.
                            assert(0);
                        }
                    }
                }
            }
        }
    }

    VkSurfaceTransformFlagBitsKHR pre_transform;
    // #TODO: Add more if necessary but identity should be enough for now
    if (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        pre_transform = caps.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[] = {
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    };
    VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
    for (uint32_t _i = 0; _i < CGPU_ARRAY_LEN(compositeAlphaFlags); _i++)
    {
        if (caps.supportedCompositeAlpha & compositeAlphaFlags[_i])
        {
            composite_alpha = compositeAlphaFlags[_i];
            break;
        }
    }
    assert(composite_alpha != VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR);

    CGpuSwapChain_Vulkan* S = (CGpuSwapChain_Vulkan*)cgpu_calloc(1, sizeof(CGpuSwapChain_Vulkan));
    S->super.device = device;
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        //
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = vkSurface,
        .minImageCount = desc->imageCount,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = sharing_mode,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &presentQueueFamilyIndex,
        .preTransform = pre_transform,
        .compositeAlpha = composite_alpha,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
        //
    };
    VkResult res = D->mVkDeviceTable.vkCreateSwapchainKHR(
        D->pVkDevice, &swapChainCreateInfo, GLOBAL_VkAllocationCallbacks, &S->pVkSwapChain);
    if (VK_SUCCESS != res)
    {
        assert(0 && "fatal: vkCreateSwapchainKHR failed!");
    }
    S->pVkSurface = vkSurface;
    return &S->super;
}

void cgpu_free_swapchain_vulkan(CGpuSwapChainId swapchain)
{
    CGpuSwapChain_Vulkan* S = (CGpuSwapChain_Vulkan*)swapchain;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)swapchain->device;

    D->mVkDeviceTable.vkDestroySwapchainKHR(D->pVkDevice, S->pVkSwapChain, GLOBAL_VkAllocationCallbacks);

    cgpu_free((void*)swapchain);
}

// exts
#include "cgpu/extensions/cgpu_vulkan_exts.h"