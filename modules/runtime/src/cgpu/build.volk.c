#include "cgpu/cgpu_config.h"
#ifdef CGPU_USE_VULKAN
    #include "cgpu/backend/vulkan/cgpu_vulkan.h" // IWYU pragma: keep
    #include "vulkan/volk.c"
    #include "shader-reflections/spirv/spirv_reflect.c"
#endif