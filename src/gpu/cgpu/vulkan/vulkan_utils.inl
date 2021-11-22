#ifdef __cplusplus
extern "C" {
#endif
// API Helpers
inline static VkBufferUsageFlags VkUtil_DescriptorTypesToBufferUsage(CGpuResourceTypes descriptors, bool texel)
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

#ifdef __cplusplus
}
#endif