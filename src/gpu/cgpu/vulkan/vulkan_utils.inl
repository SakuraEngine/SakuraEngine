#include "cgpu/flags.h"
#ifdef __cplusplus
extern "C" {
#endif
// API Helpers
FORCEINLINE static VkImageUsageFlags VkUtil_DescriptorTypesToImageUsage(CGpuResourceTypes descriptors)
{
    VkImageUsageFlags result = 0;
    if (RT_TEXTURE == (descriptors & RT_TEXTURE))
        result |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (RT_RW_TEXTURE == (descriptors & RT_RW_TEXTURE))
        result |= VK_IMAGE_USAGE_STORAGE_BIT;
    return result;
}

FORCEINLINE static VkFilter VkUtil_TranslateFilterType(ECGpuFilterType filter)
{
    switch (filter)
    {
        case FILTER_TYPE_NEAREST:
            return VK_FILTER_NEAREST;
        case FILTER_TYPE_LINEAR:
            return VK_FILTER_LINEAR;
        default:
            return VK_FILTER_LINEAR;
    }
}

FORCEINLINE static VkSamplerMipmapMode VkUtil_TranslateMipMapMode(ECGpuMipMapMode mipMapMode)
{
    switch (mipMapMode)
    {
        case MIPMAP_MODE_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case MIPMAP_MODE_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:
            cgpu_assert(false && "Invalid Mip Map Mode");
            return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
    }
}

FORCEINLINE static VkSamplerAddressMode VkUtil_TranslateAddressMode(ECGpuAddressMode addressMode)
{
    switch (addressMode)
    {
        case ADDRESS_MODE_MIRROR:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case ADDRESS_MODE_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case ADDRESS_MODE_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case ADDRESS_MODE_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

FORCEINLINE static VkImageLayout VkUtil_ResourceStateToImageLayout(ECGpuResourceState usage)
{
    if (usage & RESOURCE_STATE_COPY_SOURCE)
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    if (usage & RESOURCE_STATE_COPY_DEST)
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    if (usage & RESOURCE_STATE_RENDER_TARGET)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (usage & RESOURCE_STATE_DEPTH_WRITE)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (usage & RESOURCE_STATE_UNORDERED_ACCESS)
        return VK_IMAGE_LAYOUT_GENERAL;

    if (usage & RESOURCE_STATE_SHADER_RESOURCE)
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (usage & RESOURCE_STATE_PRESENT)
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (usage == RESOURCE_STATE_COMMON)
        return VK_IMAGE_LAYOUT_GENERAL;

#if defined(QUEST_VR)
    if (usage == RS_SHADING_RATE_SOURCE)
        return VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
#endif

    return VK_IMAGE_LAYOUT_UNDEFINED;
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

FORCEINLINE static VkBufferUsageFlags VkUtil_DescriptorTypesToBufferUsage(CGpuResourceTypes descriptors, bool texel)
{
    VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (descriptors & RT_UNIFORM_BUFFER)
    {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (descriptors & RT_RW_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (descriptors & RT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (descriptors & RT_INDEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (descriptors & RT_VERTEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (descriptors & RT_INDIRECT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
#ifdef ENABLE_RAYTRACING
    if (descriptors & RT_RAY_TRACING)
    {
        result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    }
#endif
    return result;
}

FORCEINLINE static VkShaderStageFlags VkUtil_TranslateShaderUsages(CGpuShaderStages shader_stages)
{
    VkShaderStageFlags result = 0;
    if (SHADER_STAGE_ALL_GRAPHICS == (shader_stages & SHADER_STAGE_ALL_GRAPHICS))
    {
        result = VK_SHADER_STAGE_ALL_GRAPHICS;
    }
    else
    {
        if (SHADER_STAGE_VERT == (shader_stages & SHADER_STAGE_VERT))
        {
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (SHADER_STAGE_TESC == (shader_stages & SHADER_STAGE_TESC))
        {
            result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (SHADER_STAGE_TESE == (shader_stages & SHADER_STAGE_TESE))
        {
            result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if (SHADER_STAGE_GEOM == (shader_stages & SHADER_STAGE_GEOM))
        {
            result |= VK_SHADER_STAGE_GEOMETRY_BIT;
        }
        if (SHADER_STAGE_FRAG == (shader_stages & SHADER_STAGE_FRAG))
        {
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (SHADER_STAGE_COMPUTE == (shader_stages & SHADER_STAGE_COMPUTE))
        {
            result |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }
    return result;
}

/* clang-format off */
FORCEINLINE static VkDescriptorType VkUtil_TranslateResourceType(ECGpuResourceType type)
{
	switch (type)
	{
		case RT_NONE: cgpu_assert(0 && "Invalid DescriptorInfo Type"); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		case RT_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case RT_TEXTURE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case RT_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case RT_RW_TEXTURE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case RT_BUFFER:
		case RT_RW_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case RT_INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case RT_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case RT_RW_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case RT_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#ifdef ENABLE_RAYTRACING
		case RT_RAY_TRACING: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
#endif
		default:
			cgpu_assert(0 && "Invalid DescriptorInfo Type");
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

FORCEINLINE static VkPipelineStageFlags VkUtil_DeterminePipelineStageFlags(CGpuAdapter_Vulkan* A, VkAccessFlags accessFlags, ECGpuQueueType queueType)
{
    VkPipelineStageFlags flags = 0;

	switch (queueType)
	{
		case QUEUE_TYPE_GRAPHICS:
		{
			if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

			if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
			{
				flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
				flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				if (A->adapter_detail.support_geom_shader)
				{
					flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
				}
				if (A->adapter_detail.support_tessellation)
				{
					flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
					flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
				}
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
#ifdef ENABLE_RAYTRACING
				if (pRenderer->mVulkan.mRaytracingExtension)
				{
					flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;
				}
#endif
			}
			if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
				flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		}
		case QUEUE_TYPE_COMPUTE:
		{
			if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
				(accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
				(accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
				(accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
				return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

			break;
		}
		case QUEUE_TYPE_TRANSFER: return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		default: break;
	}
	// Compatible with both compute and graphics queues
	if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
		flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

	if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_HOST_BIT;

	if (flags == 0)
		flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    return flags;
}

FORCEINLINE static VkAccessFlags VkUtil_ResourceStateToVkAccessFlags(ECGpuResourceState state)
{
	VkAccessFlags ret = 0;
	if (state & RESOURCE_STATE_COPY_SOURCE)
		ret |= VK_ACCESS_TRANSFER_READ_BIT;
	if (state & RESOURCE_STATE_COPY_DEST)
		ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
	if (state & RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
		ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	if (state & RESOURCE_STATE_INDEX_BUFFER)
		ret |= VK_ACCESS_INDEX_READ_BIT;
	if (state & RESOURCE_STATE_UNORDERED_ACCESS)
		ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	if (state & RESOURCE_STATE_INDIRECT_ARGUMENT)
		ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	if (state & RESOURCE_STATE_RENDER_TARGET)
		ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (state & RESOURCE_STATE_DEPTH_WRITE)
		ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	if (state & RESOURCE_STATE_SHADER_RESOURCE)
		ret |= VK_ACCESS_SHADER_READ_BIT;
	if (state & RESOURCE_STATE_PRESENT)
		ret |= VK_ACCESS_MEMORY_READ_BIT;
#ifdef ENABLE_RAYTRACING
	if (state & RS_RAYTRACING_ACCELERATION_STRUCTURE)
		ret |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
#endif
	return ret;
}

#define VK_OBJ_TYPE_CASE(object) case VK_OBJECT_TYPE_##object: return VK_DEBUG_REPORT_OBJECT_TYPE_##object##_EXT;
FORCEINLINE static VkDebugReportObjectTypeEXT VkUtil_ObjectTypeToDebugReportType(VkObjectType type)
{
    switch (type)
    {
        VK_OBJ_TYPE_CASE(UNKNOWN)
        VK_OBJ_TYPE_CASE(INSTANCE)
        VK_OBJ_TYPE_CASE(PHYSICAL_DEVICE)
        VK_OBJ_TYPE_CASE(DEVICE)
        VK_OBJ_TYPE_CASE(QUEUE)
        VK_OBJ_TYPE_CASE(SEMAPHORE)
        VK_OBJ_TYPE_CASE(COMMAND_BUFFER)
        VK_OBJ_TYPE_CASE(FENCE)
        VK_OBJ_TYPE_CASE(DEVICE_MEMORY)
        VK_OBJ_TYPE_CASE(BUFFER)
        VK_OBJ_TYPE_CASE(IMAGE)
        VK_OBJ_TYPE_CASE(EVENT)
        VK_OBJ_TYPE_CASE(QUERY_POOL)
        VK_OBJ_TYPE_CASE(BUFFER_VIEW)
        VK_OBJ_TYPE_CASE(IMAGE_VIEW)
        VK_OBJ_TYPE_CASE(SHADER_MODULE)
        VK_OBJ_TYPE_CASE(PIPELINE_CACHE)
        VK_OBJ_TYPE_CASE(PIPELINE_LAYOUT)
        VK_OBJ_TYPE_CASE(RENDER_PASS)
        VK_OBJ_TYPE_CASE(PIPELINE)
        VK_OBJ_TYPE_CASE(DESCRIPTOR_SET_LAYOUT)
        VK_OBJ_TYPE_CASE(SAMPLER)
        VK_OBJ_TYPE_CASE(DESCRIPTOR_POOL)
        VK_OBJ_TYPE_CASE(DESCRIPTOR_SET)
        VK_OBJ_TYPE_CASE(FRAMEBUFFER)
        VK_OBJ_TYPE_CASE(COMMAND_POOL)
        VK_OBJ_TYPE_CASE(SAMPLER_YCBCR_CONVERSION)
        VK_OBJ_TYPE_CASE(DESCRIPTOR_UPDATE_TEMPLATE)
        VK_OBJ_TYPE_CASE(SURFACE_KHR)
        VK_OBJ_TYPE_CASE(SWAPCHAIN_KHR)
        VK_OBJ_TYPE_CASE(DISPLAY_KHR)
        VK_OBJ_TYPE_CASE(DISPLAY_MODE_KHR)
        VK_OBJ_TYPE_CASE(DEBUG_REPORT_CALLBACK_EXT)
    }
    return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
}
#undef VK_OBJ_TYPE_CASE 
#ifdef __cplusplus
}
#endif