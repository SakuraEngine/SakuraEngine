#include "cgpu/cgpu_config.h"
#include "common/root_sig_table.cpp"
#include "common/root_sig_pool.cpp"
#ifdef CGPU_USE_VULKAN
    #include "vulkan/cgpu_vulkan_instance.cpp"
#endif

#ifdef CGPU_USE_D3D12
    #include "d3d12/cgpu_d3d12.cpp"
    #include "d3d12/cgpu_d3d12_rdna2.cpp"
    #include "d3d12/cgpu_d3d12_dstorage.cpp"
    #include "d3d12/d3d12_utils.cpp"
    #include "d3d12/cgpu_d3d12_surfaces.cpp"
    #include "d3d12/cgpu_d3d12_resources.cpp"
#endif

#include "common/cgpu.cpp"