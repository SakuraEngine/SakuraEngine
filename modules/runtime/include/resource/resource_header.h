#pragma once
#include "resource/resource_handle.h"
#include "type/type_registry.h"

#if defined(__cplusplus)
    #include "binary/reader.h"
    #include "binary/writer.h"
    #include "EASTL/fixed_vector.h"

typedef struct skr_resource_header_t {
    uint32_t version;
    skr_guid_t guid;
    skr_type_id_t type;
    eastl::fixed_vector<skr_resource_handle_t, 4> dependencies;
} skr_resource_header_t;
namespace skr::binary
{
    template <>
    RUNTIME_API int ReadValue(skr_binary_reader_t* reader, skr_resource_header_t& header);
    template <>
    RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_resource_header_t& header);
} // namespace skr::binary
#endif
typedef enum ESkrLoadingStatus : uint32_t
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
    #include <EASTL/vector.h>
namespace skr::resource
{
struct SResourceRequest;
}
struct skr_resource_record_t {
    void* resource;
    void (*destructor)(void*);
    ESkrLoadingStatus loadingStatus;
    struct requester_id {
        uint32_t id;
        ESkrRequesterType type;
        bool operator==(const requester_id& other) const { return id == other.id && type == other.type; };
    };
    uint32_t id;
    eastl::vector<requester_id> references;
    skr_resource_header_t header;
    skr::resource::SResourceRequest* activeRequest;
};
#endif