#include "SkrBase/types/guid.h"
#include "sole.hpp"

void skr_make_guid(skr_guid_t* out_guid)
{
    auto uuid = sole::uuid4();
    memcpy(out_guid, &uuid, sizeof(skr_guid_t));
}