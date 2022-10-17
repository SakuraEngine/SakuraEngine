#include "cgpu/cgpu_config.h"

#ifdef CGPU_USE_D3D12
    #include "d3d12/cgpu_d3d12.cpp"
    #include "d3d12/cgpu_d3d12_rdna2.cpp"
    #include "d3d12/d3d12_utils.cpp"
    #include "d3d12/cgpu_d3d12_surfaces.cpp"
    #include "d3d12/cgpu_d3d12_resources.cpp"
#endif