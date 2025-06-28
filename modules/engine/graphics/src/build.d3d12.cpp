#include "SkrGraphics/config.h"
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#endif

#ifdef CGPU_USE_D3D12
    #include <comdef.h>
    #include "d3d12/d3d12_utils.cpp"
    #include "d3d12/cgpu_d3d12.cpp"
#endif

#ifdef CGPU_USE_D3D12
    #include "d3d12/cgpu_d3d12_dred.cpp"
    #include "d3d12/cgpu_d3d12_dstorage.cpp"
#endif

#ifdef CGPU_USE_D3D12
    #ifdef SAFE_RELEASE
        #undef SAFE_RELEASE
    #endif
    #include "d3d12/D3D12MemAlloc.cpp"
#endif