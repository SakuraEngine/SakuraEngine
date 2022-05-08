#pragma once
#include "platform/guid.h"

enum ESkrLoadingStatus : uint32_t;
typedef struct skr_resource_handle_t {
    union
    {
        skr_guid_t guid;
        struct 
        {
            int32_t padding; // zero, flag for whether resolved
            int32_t requester; // requester id
            int64_t pointer; //resource record ptr
        };
    };
    #if defined(__cplusplus)
    RUNTIME_API skr_resource_handle_t();
    RUNTIME_API skr_resource_handle_t(const skr_resource_handle_t& other);
    RUNTIME_API skr_resource_handle_t(skr_resource_handle_t&& other);
    RUNTIME_API void set_ptr(void* ptr);
    RUNTIME_API void set_guid(const skr_guid_t& guid);
    RUNTIME_API bool is_resolved() const;
    RUNTIME_API void* get_resolved(bool requireInstalled = true) const;
    RUNTIME_API skr_guid_t get_serialized() const;
    RUNTIME_API void resolve(bool requireInstalled = true, uint32_t requester = 0);
    RUNTIME_API void serialize();
    RUNTIME_API skr_guid_t get_guid() const;
    RUNTIME_API void* get_ptr() const;
    RUNTIME_API bool is_null() const;
    RUNTIME_API void reset();
    RUNTIME_API ESkrLoadingStatus get_status();
    #endif
} skr_resource_handle_t;

#if defined(__cplusplus)
namespace skr::resource
{
    template<class T>
    struct TResourceHandle : skr_resource_handle_t
    {

    };
}
#endif

RUNTIME_API int skr_is_resource_resolved(skr_resource_handle_t* handle);
RUNTIME_API void skr_get_resource_guid(skr_resource_handle_t* handle, skr_guid_t* guid);
RUNTIME_API void skr_get_resource(skr_resource_handle_t* handle, void** guid);
