#include "../common/common_utils.h"
#include "cgpu/backend/metal/cgpu_metal.h"

const CGPUSurfacesProcTable s_tbl_metal = {
    //
    .free_surface = NULL,
#if defined(_MACOS)
    .from_ns_view = NULL
#endif
    //
};

const CGPUSurfacesProcTable* CGPU_MetalSurfacesProcTable()
{
    return &s_tbl_metal;
};
