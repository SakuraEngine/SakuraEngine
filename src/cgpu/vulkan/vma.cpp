#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused"
#endif

#define VMA_IMPLEMENTATION
#define WIN32_MEAN_AND_LEAN
#define VMA_RECORDING_ENABLED 0
#define VMA_STATS_STRING_ENABLED 0
#define VMA_VULKAN_VERSION 1002000 // stay at vk 1.2 now
#include "cgpu/backend/vulkan/vk_mem_alloc.h"

#ifdef __clang__
    #pragma clang diagnostic pop
#endif