#include "cgpu/backend/metal/cgpu_metal.h"

const CGpuSurfacesProcTable s_tbl_metal = {
    //
    .free_surface = NULL,
#if defined(_MACOS)
    .from_ns_view = NULL
#endif
    //
};

const CGpuSurfacesProcTable* CGPU_MetalSurfacesProcTable()
{
    return &s_tbl_metal;
};
