#include "SkrRT/rttr/guid.hpp"
#include "SkrRT/misc/types.h"

namespace skr
{
GUID GUID::Create()
{
    GUID id;
    skr_make_guid(reinterpret_cast<skr_guid_t*>(&id));
    return id;
}
} // namespace skr