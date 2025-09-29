#include "SkrBase/config.h"
#include "SkrBase/types/guid.hpp"

#if SKR_PLAT_WINDOWS
    #include <combaseapi.h>

static void _make_guid(::skr::GUID* out)
{
    CoCreateGuid(reinterpret_cast<::GUID*>(out));
}
#elif SKR_PLAT_MACOSX
    #include <CoreFoundation/CFUUID.h>

static void _make_guid(::skr::GUID* out)
{
    auto new_id = CFUUIDCreate(NULL);
    auto bytes  = CFUUIDGetUUIDBytes(new_id);
    CFRelease(new_id);
    memcpy(out, &bytes, sizeof(::skr::GUID));
}

#else
    #pragma error "Unsupported platform"
#endif

// guid
namespace skr
{
// create new
void GUID::create()
{
    _make_guid(this);
}
} // namespace skr