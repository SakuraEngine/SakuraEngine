#include "log.c"
#include "utils/types.h"

extern void dual_make_guid(skr_guid_t* guid);

RUNTIME_API void skr_make_guid(skr_guid_t* out_guid)
{
    dual_make_guid(out_guid);
}