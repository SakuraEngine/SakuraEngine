#include "cgpu/cgpu_config.h"
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#endif

#ifdef CGPU_USE_VULKAN
    #include "vulkan/vma.cpp"
#endif
