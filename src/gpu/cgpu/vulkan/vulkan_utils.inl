#ifdef __cplusplus
extern "C" {
#endif
// API Helpers
inline static VkBufferUsageFlags VkUtil_DescriptorTypesToBufferUsage(CGpuDescriptorTypes descriptors, bool texel)
{
    VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (descriptors & DT_UNIFORM_BUFFER)
    {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (descriptors & DT_RW_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (descriptors & DT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (descriptors & DT_INDEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (descriptors & DT_VERTEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (descriptors & DT_INDIRECT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
#ifdef ENABLE_RAYTRACING
    if (descriptors & DT_RAY_TRACING)
    {
        result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    }
#endif
    return result;
}

#ifdef __cplusplus
}
#endif