#include "cgpu/cgpu_config.h"
#ifdef CGPU_USE_VULKAN
    #include "vulkan/vulkan_utils.c"
    #include "vulkan/proc_table.c"
    #include "vulkan/cgpu_vulkan.c"
    #include "vulkan/cgpu_vulkan2.c"
    #include "vulkan/cgpu_vulkan_rdna2.c"
    #include "vulkan/cgpu_vulkan_resources.c"
    #include "vulkan/cgpu_vulkan_surfaces.c"
#endif
#ifdef CGPU_USE_D3D12
    #include "d3d12/proc_table.c"
#endif
#include "common/cgpu.c"