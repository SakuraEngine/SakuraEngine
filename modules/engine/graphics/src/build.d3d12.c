#include "SkrGraphics/config.h"
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#endif

#ifdef CGPU_USE_D3D12
    #include "d3d12/cgpu_d3d12.c"
    #include "d3d12/cgpu_d3d12_raytracing.c"
    #include "d3d12/cgpu_d3d12_surfaces.c"
    #include "d3d12/cgpu_d3d12_rdna2.c"
#endif