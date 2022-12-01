#include "cgpu/cgpu_config.h"
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#endif

#ifdef CGPU_USE_D3D12
    #ifdef SAFE_RELEASE
        #undef SAFE_RELEASE
    #endif
    #include "d3d12/D3D12MemAlloc.cpp"
#endif