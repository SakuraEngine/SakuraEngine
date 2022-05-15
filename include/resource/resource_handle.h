#pragma once
#include "platform/guid.h"

enum ESkrLoadingStatus : uint32_t;
struct skr_resource_record_t;
enum ESkrRequesterType
{
    SKR_REQUESTER_ENTITY = 0,
    SKR_REQUESTER_DEPENDENCY = 1,
    SKR_REQUESTER_SYSTEM = 2,
    SKR_REQUESTER_UNKNOWN = 3
};
typedef struct skr_resource_handle_t {
    union
    {
        skr_guid_t guid;
        struct
        {
            uint32_t padding;   // zero, flag for whether resolved
            uint32_t requester; // requester id
            // since resource record is allocated with alignment 8, the lower 3 bits should always be zero
            // so we put requester type into it
            uint64_t pointer; // resource record ptr & requester type
        };
    };
#if defined(__cplusplus)
    RUNTIME_API skr_resource_handle_t();
    RUNTIME_API ~skr_resource_handle_t();
    RUNTIME_API skr_resource_handle_t(const skr_guid_t& other);
    RUNTIME_API skr_resource_handle_t(const skr_resource_handle_t& other);
    RUNTIME_API skr_resource_handle_t(skr_resource_handle_t&& other);
    RUNTIME_API skr_resource_handle_t& operator=(const skr_resource_handle_t& other);
    RUNTIME_API skr_resource_handle_t& operator=(const skr_guid_t& other);
    RUNTIME_API skr_resource_handle_t& operator=(skr_resource_handle_t&& other);
    RUNTIME_API void set_ptr(void* ptr);
    RUNTIME_API void set_guid(const skr_guid_t& guid);
    RUNTIME_API bool is_resolved() const;
    RUNTIME_API void* get_resolved(bool requireInstalled = true) const;
    RUNTIME_API skr_guid_t get_serialized() const;
    RUNTIME_API void resolve(bool requireInstalled = true, uint32_t requester = 0, ESkrRequesterType requesterType = SKR_REQUESTER_UNKNOWN);
    RUNTIME_API void unload();
    RUNTIME_API skr_guid_t get_guid() const;
    RUNTIME_API void* get_ptr() const;
    RUNTIME_API bool is_null() const;
    RUNTIME_API void reset();
    RUNTIME_API uint32_t get_requester() const;
    RUNTIME_API ESkrRequesterType get_requester_type() const;
    RUNTIME_API ESkrLoadingStatus get_status() const;
    RUNTIME_API skr_resource_record_t* get_record() const;
    RUNTIME_API void set_record(skr_resource_record_t* record);
    RUNTIME_API void set_resolved(skr_resource_record_t* record, uint32_t requester, ESkrRequesterType requesterType);
#endif
} skr_resource_handle_t;

#if defined(__cplusplus)
namespace skr::resource
{
template <class T>
struct TResourceHandle : skr_resource_handle_t {
};
} // namespace skr::resource
#endif

RUNTIME_API int skr_is_resource_resolved(skr_resource_handle_t* handle);
RUNTIME_API void skr_get_resource_guid(skr_resource_handle_t* handle, skr_guid_t* guid);
RUNTIME_API void skr_get_resource(skr_resource_handle_t* handle, void** guid);
