#include "SkrBase/config.h"
#include "SkrBase/types/guid.h"

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
GUID GUID::Create()
{
    GUID out;
    _make_guid(&out);
    return out;
}
} // namespace skr

// guid capi
SKR_EXTERN_C void skr_create_guid(skr_guid_t* out_guid)
{
    _make_guid(out_guid);
}
SKR_EXTERN_C bool skr_decode_guid(skr_guid_t* out_guid, const skr_char8* str, uint64_t len)
{
    auto opt_guid = ::skr::GUID::DecodeAuto(str, len);
    if (opt_guid.has_value()) [[likely]]
    {
        *out_guid = opt_guid.value();
        return true;
    }
    else
    {
        return false;
    }
}
