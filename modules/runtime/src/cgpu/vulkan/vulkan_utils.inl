#include "cgpu/flags.h"
#include "cgpu/backend/vulkan/cgpu_vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif
SKR_UNUSED static const VkCullModeFlagBits gVkCullModeTranslator[CGPU_CULL_MODE_COUNT] = {
    VK_CULL_MODE_NONE,
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_BIT
};

SKR_UNUSED static const VkPolygonMode gVkFillModeTranslator[CGPU_FILL_MODE_COUNT] = {
    VK_POLYGON_MODE_FILL,
    VK_POLYGON_MODE_LINE
};

SKR_UNUSED static const VkFrontFace gVkFrontFaceTranslator[] = {
    VK_FRONT_FACE_COUNTER_CLOCKWISE,
    VK_FRONT_FACE_CLOCKWISE
};

SKR_UNUSED static const VkBlendFactor gVkBlendConstantTranslator[CGPU_BLEND_CONST_COUNT] = {
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ONE,
    VK_BLEND_FACTOR_SRC_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    VK_BLEND_FACTOR_DST_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    VK_BLEND_FACTOR_SRC_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    VK_BLEND_FACTOR_DST_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
    VK_BLEND_FACTOR_CONSTANT_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
};

SKR_UNUSED static const VkBlendOp gVkBlendOpTranslator[CGPU_BLEND_MODE_COUNT] = {
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
    VK_BLEND_OP_REVERSE_SUBTRACT,
    VK_BLEND_OP_MIN,
    VK_BLEND_OP_MAX,
};

// API Helpers
SKR_FORCEINLINE static void VkUtil_GetVertexInputBindingAttrCount(const CGPUVertexLayout* pLayout, uint32_t* pBindingCount, uint32_t* pAttrCount)
{
    uint32_t input_binding_count = 0;
	uint32_t input_attribute_count = 0;
    if (pLayout != NULL)
    {
        // Ignore everything that's beyond CGPU_MAX_VERTEX_ATTRIBS
        uint32_t attrib_count = pLayout->attribute_count > CGPU_MAX_VERTEX_ATTRIBS ? CGPU_MAX_VERTEX_ATTRIBS : pLayout->attribute_count;
        uint32_t binding_value = UINT32_MAX;
        // Initial values
        for (uint32_t i = 0; i < attrib_count; ++i)
        {
            const CGPUVertexAttribute* attrib = &(pLayout->attributes[i]);
            const uint32_t array_size = attrib->array_size ? attrib->array_size : 1;
            if (binding_value != attrib->binding)
            {
                binding_value = attrib->binding;
                input_binding_count += 1;
            }
            for(uint32_t j = 0; j < array_size; j++)
            {
                input_attribute_count += 1;
            }
        }
    }
    if (pBindingCount) *pBindingCount = input_binding_count;
    if (pAttrCount) *pAttrCount = input_attribute_count;
}

SKR_FORCEINLINE static VkPrimitiveTopology VkUtil_TranslateTopology(ECGPUPrimitiveTopology prim_topology)
{
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    switch (prim_topology)
    {
        case CGPU_PRIM_TOPO_POINT_LIST: topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
        case CGPU_PRIM_TOPO_LINE_LIST: topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
        case CGPU_PRIM_TOPO_LINE_STRIP: topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; break;
        case CGPU_PRIM_TOPO_TRI_STRIP: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; break;
        case CGPU_PRIM_TOPO_PATCH_LIST: topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST; break;
        case CGPU_PRIM_TOPO_TRI_LIST: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
        default:  cgpu_assert(false && "Primitive Topo not supported!"); break;
    }
    return topology;
}

SKR_FORCEINLINE static VkImageUsageFlags VkUtil_DescriptorTypesToImageUsage(CGPUResourceTypes descriptors)
{
    VkImageUsageFlags result = 0;
    if (CGPU_RESOURCE_TYPE_TEXTURE == (descriptors & CGPU_RESOURCE_TYPE_TEXTURE))
        result |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (CGPU_RESOURCE_TYPE_RW_TEXTURE == (descriptors & CGPU_RESOURCE_TYPE_RW_TEXTURE))
        result |= VK_IMAGE_USAGE_STORAGE_BIT;
    return result;
}

SKR_FORCEINLINE static VkFilter VkUtil_TranslateFilterType(ECGPUFilterType filter)
{
    switch (filter)
    {
        case CGPU_FILTER_TYPE_NEAREST:
            return VK_FILTER_NEAREST;
        case CGPU_FILTER_TYPE_LINEAR:
            return VK_FILTER_LINEAR;
        default:
            return VK_FILTER_LINEAR;
    }
}

SKR_FORCEINLINE static VkSamplerMipmapMode VkUtil_TranslateMipMapMode(ECGPUMipMapMode mipMapMode)
{
    switch (mipMapMode)
    {
        case CGPU_MIPMAP_MODE_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case CGPU_MIPMAP_MODE_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:
            cgpu_assert(false && "Invalid Mip Map Mode");
            return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
    }
}

SKR_FORCEINLINE static VkSamplerAddressMode VkUtil_TranslateAddressMode(ECGPUAddressMode addressMode)
{
    switch (addressMode)
    {
        case CGPU_ADDRESS_MODE_MIRROR:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case CGPU_ADDRESS_MODE_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case CGPU_ADDRESS_MODE_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case CGPU_ADDRESS_MODE_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

SKR_FORCEINLINE static VkImageLayout VkUtil_ResourceStateToImageLayout(ECGPUResourceState usage)
{
    if (usage & CGPU_RESOURCE_STATE_COPY_SOURCE)
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    if (usage & CGPU_RESOURCE_STATE_COPY_DEST)
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    if (usage & CGPU_RESOURCE_STATE_RENDER_TARGET)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (usage & CGPU_RESOURCE_STATE_RESOLVE_DEST)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (usage & CGPU_RESOURCE_STATE_DEPTH_WRITE)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (usage & CGPU_RESOURCE_STATE_UNORDERED_ACCESS)
        return VK_IMAGE_LAYOUT_GENERAL;

    if (usage & CGPU_RESOURCE_STATE_SHADER_RESOURCE)
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (usage & CGPU_RESOURCE_STATE_PRESENT)
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (usage == CGPU_RESOURCE_STATE_COMMON)
        return VK_IMAGE_LAYOUT_GENERAL;

    if (usage == CGPU_RESOURCE_STATE_SHADING_RATE_SOURCE)
        return VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

SKR_FORCEINLINE static VkImageAspectFlags VkUtil_DeterminAspectMask(VkFormat format, bool includeStencilBit)
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

SKR_FORCEINLINE static VkBufferUsageFlags VkUtil_DescriptorTypesToBufferUsage(CGPUResourceTypes descriptors, bool texel)
{
    VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
    {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (descriptors & CGPU_RESOURCE_TYPE_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (descriptors & CGPU_RESOURCE_TYPE_INDEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (descriptors & CGPU_RESOURCE_TYPE_VERTEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (descriptors & CGPU_RESOURCE_TYPE_INDIRECT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
#ifdef ENABLE_RAYTRACING
    if (descriptors & CGPU_RESOURCE_TYPE_RAY_TRACING)
    {
        result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    }
#endif
    return result;
}

SKR_FORCEINLINE static VkShaderStageFlags VkUtil_TranslateShaderUsages(CGPUShaderStages shader_stages)
{
    VkShaderStageFlags result = 0;
    if (CGPU_SHADER_STAGE_ALL_GRAPHICS == (shader_stages & CGPU_SHADER_STAGE_ALL_GRAPHICS))
    {
        result = VK_SHADER_STAGE_ALL_GRAPHICS;
    }
    else
    {
        if (CGPU_SHADER_STAGE_VERT == (shader_stages & CGPU_SHADER_STAGE_VERT))
        {
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (CGPU_SHADER_STAGE_TESC == (shader_stages & CGPU_SHADER_STAGE_TESC))
        {
            result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (CGPU_SHADER_STAGE_TESE == (shader_stages & CGPU_SHADER_STAGE_TESE))
        {
            result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if (CGPU_SHADER_STAGE_GEOM == (shader_stages & CGPU_SHADER_STAGE_GEOM))
        {
            result |= VK_SHADER_STAGE_GEOMETRY_BIT;
        }
        if (CGPU_SHADER_STAGE_FRAG == (shader_stages & CGPU_SHADER_STAGE_FRAG))
        {
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (CGPU_SHADER_STAGE_COMPUTE == (shader_stages & CGPU_SHADER_STAGE_COMPUTE))
        {
            result |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }
    return result;
}

SKR_FORCEINLINE static uint32_t VkUtil_GetShadingRateX(ECGPUShadingRate shading_rate)
{
    switch (shading_rate)
    {
        case CGPU_SHADING_RATE_FULL:
        case CGPU_SHADING_RATE_1X2:
            return 1;
        case CGPU_SHADING_RATE_HALF:
        case CGPU_SHADING_RATE_2X1:
        case CGPU_SHADING_RATE_2X4:
            return 2;
        case CGPU_SHADING_RATE_QUARTER:
        case CGPU_SHADING_RATE_4X2:
            return 4;
        default:
            return 1;
    }
}

SKR_FORCEINLINE static uint32_t VkUtil_GetShadingRateY(ECGPUShadingRate shading_rate)
{
    switch (shading_rate)
    {
        case CGPU_SHADING_RATE_FULL:
        case CGPU_SHADING_RATE_2X1:
            return 1;
        case CGPU_SHADING_RATE_HALF:
        case CGPU_SHADING_RATE_1X2:
        case CGPU_SHADING_RATE_4X2:
            return 2;
        case CGPU_SHADING_RATE_QUARTER:
        case CGPU_SHADING_RATE_2X4:
            return 4;
        default:
            return 1;
    }
}

SKR_FORCEINLINE static VkFragmentShadingRateCombinerOpKHR VkUtil_TranslateShadingRateCombiner(ECGPUShadingRateCombiner combiner)
{
    switch (combiner)
    {
        case CGPU_SHADING_RATE_COMBINER_PASSTHROUGH:
            return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
        case CGPU_SHADING_RATE_COMBINER_OVERRIDE:
            return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
        case CGPU_SHADING_RATE_COMBINER_MIN:
            return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MIN_KHR;
        case CGPU_SHADING_RATE_COMBINER_MAX:
            return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR;
        case CGPU_SHADING_RATE_COMBINER_SUM:
            return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR;
        default:
            return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    }
}

/* clang-format off */
SKR_FORCEINLINE static VkDescriptorType VkUtil_TranslateResourceType(ECGPUResourceType type)
{
	switch (type)
	{
		case CGPU_RESOURCE_TYPE_NONE: cgpu_assert(0 && "Invalid DescriptorInfo Type"); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		case CGPU_RESOURCE_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case CGPU_RESOURCE_TYPE_TEXTURE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case CGPU_RESOURCE_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case CGPU_RESOURCE_TYPE_RW_TEXTURE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case CGPU_RESOURCE_TYPE_BUFFER:
		case CGPU_RESOURCE_TYPE_RW_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case CGPU_RESOURCE_TYPE_INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case CGPU_RESOURCE_TYPE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case CGPU_RESOURCE_TYPE_RW_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case CGPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#ifdef ENABLE_RAYTRACING
		case CGPU_RESOURCE_TYPE_RAY_TRACING: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
#endif
		default:
			cgpu_assert(0 && "Invalid DescriptorInfo Type");
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

SKR_FORCEINLINE static VkPipelineStageFlags VkUtil_DeterminePipelineStageFlags(CGPUAdapter_Vulkan* A, VkAccessFlags accessFlags, ECGPUQueueType queue_type)
{
    VkPipelineStageFlags flags = 0;

	switch (queue_type)
	{
		case CGPU_QUEUE_TYPE_GRAPHICS:
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
		case CGPU_QUEUE_TYPE_COMPUTE:
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
		case CGPU_QUEUE_TYPE_TRANSFER: return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
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

SKR_FORCEINLINE static VkAccessFlags VkUtil_ResourceStateToVkAccessFlags(ECGPUResourceState state)
{
	VkAccessFlags ret = VK_ACCESS_NONE;
	if (state & CGPU_RESOURCE_STATE_COPY_SOURCE)
		ret |= VK_ACCESS_TRANSFER_READ_BIT;
	if (state & CGPU_RESOURCE_STATE_COPY_DEST)
		ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
	if (state & CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
		ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	if (state & CGPU_RESOURCE_STATE_INDEX_BUFFER)
		ret |= VK_ACCESS_INDEX_READ_BIT;
	if (state & CGPU_RESOURCE_STATE_UNORDERED_ACCESS)
		ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	if (state & CGPU_RESOURCE_STATE_INDIRECT_ARGUMENT)
		ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	if (state & CGPU_RESOURCE_STATE_RENDER_TARGET)
		ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (state & CGPU_RESOURCE_STATE_RESOLVE_DEST)
		ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (state & CGPU_RESOURCE_STATE_DEPTH_WRITE)
		ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	if (state & CGPU_RESOURCE_STATE_SHADER_RESOURCE)
		ret |= VK_ACCESS_SHADER_READ_BIT;
	if (state & CGPU_RESOURCE_STATE_PRESENT)
		ret |= VK_ACCESS_NONE;
#ifdef ENABLE_RAYTRACING
	if (state & CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE)
		ret |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
#endif
	return ret;
}

#define VK_OBJ_TYPE_CASE(object) case VK_OBJECT_TYPE_##object: return VK_DEBUG_REPORT_OBJECT_TYPE_##object##_EXT;
SKR_FORCEINLINE static VkDebugReportObjectTypeEXT VkUtil_ObjectTypeToDebugReportType(VkObjectType type)
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
    default: return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    }
    return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
}
#undef VK_OBJ_TYPE_CASE 
#ifdef __cplusplus
}
#endif