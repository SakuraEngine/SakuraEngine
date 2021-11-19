#include "metal_utils.h"
#import "cgpu/backend/metal/cgpu_metal_types.h"

void MetalUtil_EnumFormatSupports(struct CGpuAdapter_Metal* MAdapter)
{
    for (uint32_t i = 0; i < PF_Count; ++i)
    {
        MAdapter->super.format_supports[i].shader_read = 0;
        MAdapter->super.format_supports[i].shader_write = 0;
        MAdapter->super.format_supports[i].render_target_write = 0;
    }

    return;
}