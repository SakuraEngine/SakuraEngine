#pragma once

// select skr_guid_t implementation
#ifdef __cplusplus
    #include "./impl/guid.hpp" // IWYU pragma: export
using skr_guid_t = ::skr::GUID;
#else
typedef struct skr_guid_t
{
    uint32_t storage0;
    uint32_t storage1;
    uint32_t storage2;
    uint32_t storage3;
} skr_guid_t;
#endif

SKR_EXTERN_C void skr_create_guid(skr_guid_t* out_guid);
SKR_EXTERN_C bool skr_decode_guid(skr_guid_t* out_guid, const skr_char8* str, uint64_t len);