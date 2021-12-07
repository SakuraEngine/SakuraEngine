#include "cgpu/flags.h"
#ifdef __cplusplus
extern "C" {
#endif
// API Helpers
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

FORCEINLINE static VkShaderStageFlags VkUtil_TranslateShaderUsages(CGpuShaderStages stages)
{
    VkShaderStageFlags res = 0;
    if (stages & SS_ALL_GRAPHICS)
        return VK_SHADER_STAGE_ALL_GRAPHICS;

    if (stages & SS_VERT)
        res |= VK_SHADER_STAGE_VERTEX_BIT;
    if (stages & SS_GEOM)
        res |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (stages & SS_TESE)
        res |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (stages & SS_TESC)
        res |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (stages & SS_COMPUTE)
        res |= VK_SHADER_STAGE_COMPUTE_BIT;
#ifdef ENABLE_RAYTRACING
    if (stages & SS_RAYTRACING)
        res |=
            (VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV |
                VK_SHADER_STAGE_MISS_BIT_NV | VK_SHADER_STAGE_INTERSECTION_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV);
#endif
    assert(res != 0);
    return res;
}

/* clang-format off */
FORCEINLINE static VkDescriptorType VkUtil_TranslateResourceType(ECGpuResourceType type)
{
	switch (type)
	{
		case RT_NONE: assert(0 && "Invalid DescriptorInfo Type"); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
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
			assert(0 && "Invalid DescriptorInfo Type");
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
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