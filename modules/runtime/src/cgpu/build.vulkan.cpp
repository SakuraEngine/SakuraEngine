#include "cgpu/cgpu_config.h"
#include "common/root_sig_table.cpp"
#include "common/root_sig_pool.cpp"

#ifdef CGPU_USE_VULKAN
    #include "vulkan/cgpu_vulkan_instance.cpp"
    #ifdef _WIN32
        #include "vulkan/cgpu_vulkan_windows.cpp"
    #endif
#endif

// common utils & ags & nvapi
#include "common/cgpu.cpp"
#include "common/cgpu_ags.cpp"
#include "common/cgpu_nvapi.cpp"

// marker buffers
#include "extensions/marker_buffer.cpp"
#ifdef CGPU_USE_VULKAN
#include "extensions/marker_buffer_vulkan.cpp"
#endif
