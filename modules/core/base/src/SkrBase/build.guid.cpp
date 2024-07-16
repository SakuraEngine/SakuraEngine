#include "SkrBase/config.h"
#include "SkrBase/types/guid.h"

#if SKR_PLAT_WINDOWS

#include <combaseapi.h>

void skr_make_guid(skr_guid_t* out_guid)
{
    GUID gidReference;
    CoCreateGuid( &gidReference );
    memcpy(out_guid, &gidReference, sizeof(skr_guid_t));
}

#elif SKR_PLAT_MACOSX

#include <CoreFoundation/CFUUID.h>

void skr_make_guid(skr_guid_t* out_guid)
{
    auto newId = CFUUIDCreate(NULL);
	auto bytes = CFUUIDGetUUIDBytes(newId);
	CFRelease(newId);
    memcpy(out_guid, &bytes, sizeof(skr_guid_t));
}

#else

#pragma error "Unsupported platform"

#endif