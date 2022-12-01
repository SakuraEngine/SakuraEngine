#include "cgpu/cgpu_config.h"

#ifdef CGPU_USE_VULKAN
    #include "vulkan/cgpu_vulkan_instance.cpp"
    #ifdef _WIN32
        #include "vulkan/cgpu_vulkan_windows.cpp"
    #endif
#endif

// marker buffers
#ifdef CGPU_USE_VULKAN
#include "extensions/marker_buffer_vulkan.cpp"
#endif
