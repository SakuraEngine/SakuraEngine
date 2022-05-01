#pragma once
#include "platform/guid.h"
#include "type/type_registry.h"

typedef struct skr_resource_handle_t {
    union
    {
        skr_guid_t guid;
        void* pointer;
    };
} skr_resource_handle_t;

typedef struct skr_resource_header_t skr_resource_header_t;

#if defined(__cplusplus)
    #include "bitsery/brief_syntax.h"
    #include "bitsery/ext/growable.h"
    #include "EASTL/fixed_vector.h"

typedef struct skr_resource_header_t {
    skr_guid_t guid;
    uint32_t version;
    skr_type_id_t type;
    eastl::fixed_vector<skr_guid_t, 4> dependencies;
} skr_resource_header_t;
namespace bitsery
{
template <class S>
void serialize(S& s, skr_resource_header_t& header)
{
    uint32_t version = 0; // version of this function
    s.value4b(version);
    s.value4b(header.version);
    s.value16b(header.guid);
    s.value16b(header.type);
    s.container16b(header.dependencies);
}
} // namespace bitsery
#endif
typedef enum ESkrLoadingStatus
{
    SKR_LOADING_STATUS_UNLOADED,
    SKR_LOADING_STATUS_LOADING,    // file io & deserialize
    SKR_LOADING_STATUS_LOADED,     // resource data is deserialized
    SKR_LOADING_STATUS_INSTALLING, // waiting dependencies & initializing
    SKR_LOADING_STATUS_INSTALLED,  // resource is fully initialized
    SKR_LOADING_STATUS_UNINSTALLING,
    SKR_LOADING_STATUS_UNLOADING,
    SKR_LOADING_STATUS_ERROR,
} ESkrLoadingStatus;

typedef struct skr_resource_record_t skr_resource_record_t;

#if defined(__cplusplus)
    #include "EASTL/vector.h"
extern "C" struct skr_resource_record_t {
    void* resource;
    void (*destructor)(void*);
    ESkrLoadingStatus loadingStatus;
    eastl::vector<uint64_t> references;
    skr_resource_header_t header;
};
#endif