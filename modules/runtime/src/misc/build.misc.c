#include "SkrRT/misc/types.h"

extern void sugoi_make_guid(skr_guid_t* guid);

SKR_RUNTIME_API void skr_make_guid(skr_guid_t* out_guid)
{
    sugoi_make_guid(out_guid);
}