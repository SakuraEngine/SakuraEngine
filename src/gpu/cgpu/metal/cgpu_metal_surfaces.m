#include "cgpu/backend/metal/cgpu_metal.h"

const CGpuSurfacesProcTable s_tbl_metal = 
{
#if defined (_MACOS)
	.from_ns_view = NULL,
#endif
    .free_surface = NULL
};

const CGpuSurfacesProcTable* CGPU_MetalSurfacesProcTable()
{
    return &s_tbl_metal;
};
