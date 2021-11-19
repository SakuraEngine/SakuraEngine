#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused"
#endif

#define VMA_IMPLEMENTATION
#define WIN32_MEAN_AND_LEAN
#define VMA_RECORDING_ENABLED 0
#define VMA_USE_STL_UNORDERED_MAP 0 // unused
#define VMA_USE_STL_LIST 0          // off
#define VMA_USE_STL_VECTOR 0        // off
#include "cgpu/backend/vulkan/vk_mem_alloc.h"

#ifdef __clang__
    #pragma clang diagnostic pop
#endif