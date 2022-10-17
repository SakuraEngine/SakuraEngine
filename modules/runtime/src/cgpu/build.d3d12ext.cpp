#include "cgpu/cgpu_config.h"

#ifdef CGPU_USE_D3D12
#include "extensions/marker_buffer_d3d12.cpp"
#endif

#ifdef CGPU_USE_D3D12
    #include "d3d12/cgpu_d3d12_dred.cpp"
    #include "d3d12/cgpu_d3d12_dstorage.cpp"
#endif