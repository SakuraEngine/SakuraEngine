#pragma once
#include "SkrBase/config/key_words.h"
#include <SkrBase/type_info.hpp>
#include "SkrCore/log.hpp"

enum ESkrLoadingStatus : uint32_t;
struct SResourceRecord;
enum ESkrRequesterType
{
    SKR_REQUESTER_ENTITY = 0,
    SKR_REQUESTER_DEPENDENCY = 1,
    SKR_REQUESTER_SYSTEM = 2,
    SKR_REQUESTER_SCRIPT = 3,
    SKR_REQUESTER_UNKNOWN = 4
};
struct lua_State;
typedef struct SResourceHandle
{
#if defined(__cplusplus)
public:
    SKR_RUNTIME_API SResourceHandle();
    SKR_RUNTIME_API SResourceHandle(const skr::GUID& other);
    SKR_RUNTIME_API SResourceHandle(const SResourceHandle& other);
    SKR_RUNTIME_API SResourceHandle(const SResourceHandle& other, uint64_t requester, ESkrRequesterType requesterType);
    SKR_RUNTIME_API SResourceHandle(SResourceHandle&& other);
    SKR_RUNTIME_API ~SResourceHandle();
    SKR_RUNTIME_API SResourceHandle& operator=(const SResourceHandle& other);
    SKR_RUNTIME_API SResourceHandle& operator=(const skr::GUID& other);
    SKR_RUNTIME_API SResourceHandle& operator=(SResourceHandle&& other);
    SKR_RUNTIME_API void set_ptr(void* ptr);
    SKR_RUNTIME_API void set_guid(const skr::GUID& guid);
    SKR_RUNTIME_API bool is_resolved() const;
    SKR_RUNTIME_API void* get_resolved(bool requireInstalled = true) const;
    SKR_RUNTIME_API skr::GUID get_serialized() const;
    SKR_RUNTIME_API void resolve(bool requireInstalled, uint64_t requester, ESkrRequesterType requesterType);
    void resolve(bool requireInstalled, struct sugoi_storage_t* requester)
    {
        resolve(requireInstalled, (uint64_t)requester, SKR_REQUESTER_ENTITY);
    }
    SKR_RUNTIME_API void unload();
    SKR_RUNTIME_API skr::GUID get_guid() const;
    SKR_RUNTIME_API skr::GUID get_type() const;
    SKR_RUNTIME_API void* get_ptr() const;
    SKR_RUNTIME_API bool is_null() const;
    SKR_RUNTIME_API void reset();
    SResourceHandle clone(uint64_t requester, ESkrRequesterType requesterType)
    {
        return { *this, requester, requesterType };
    }
    SResourceHandle clone(struct sugoi_storage_t* requester)
    {
        return { *this, (uint64_t)requester, SKR_REQUESTER_ENTITY };
    }
    SKR_RUNTIME_API uint32_t get_requester_id() const;
    SKR_RUNTIME_API ESkrRequesterType get_requester_type() const;
    // if resolve is false, then unresolve handle will always return SKR_LOADING_STATUS_UNLOADED
    SKR_RUNTIME_API ESkrLoadingStatus get_status(bool resolve = false) const;
    SKR_RUNTIME_API SResourceRecord* get_record() const;
    SKR_RUNTIME_API void set_record(SResourceRecord* record);
    SKR_RUNTIME_API void set_resolved(SResourceRecord* record, uint32_t requesterId, ESkrRequesterType requesterType);

protected:
#endif
    union
    {
        skr_guid_t guid;
        struct
        {
            uint32_t padding;     // zero, flag for resolved or not
            uint32_t requesterId; // requester id
            // since resource record is allocated with alignment 8, the lower 3 bits should always be zero
            // so we put requester type into it
            uint64_t pointer; // resource record ptr & requester type
        };
    };
} SResourceHandle;

SKR_EXTERN_C SKR_RUNTIME_API int skr_is_resource_resolved(SResourceHandle* handle);
SKR_EXTERN_C SKR_RUNTIME_API void skr_get_resource_guid(SResourceHandle* handle, skr_guid_t* guid);
SKR_EXTERN_C SKR_RUNTIME_API void skr_get_resource(SResourceHandle* handle, void** guid);

SKR_TYPE_INFO(SResourceHandle, "A9E0CE3D-5E9B-45F1-AC28-B882885C63AB");
namespace skr
{
using ResourceHandle = SResourceHandle;
template <class T>
struct AsyncResource : ResourceHandle
{
    AsyncResource() SKR_NOEXCEPT;
    AsyncResource(const skr::GUID& guid) SKR_NOEXCEPT;

    T* get_resolved(bool requireInstalled = true) const
    {
        return (T*)ResourceHandle::get_resolved(requireInstalled);
    }
    T* get_ptr() const
    {
        return (T*)ResourceHandle::get_ptr();
    }
    AsyncResource clone(uint64_t requester, ESkrRequesterType requesterType)
    {
        return { *this, requester, requesterType };
    }
    AsyncResource clone(struct sugoi_storage_t* requester)
    {
        return { *this, (uint64_t)requester, SKR_REQUESTER_ENTITY };
    }
};

template <class T>
inline AsyncResource<T>::AsyncResource() SKR_NOEXCEPT
    : ResourceHandle()
{
}

template <class T>
inline AsyncResource<T>::AsyncResource(const skr::GUID& guid) SKR_NOEXCEPT
    : ResourceHandle(guid)
{
}
} // namespace skr

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <>
struct Serialize<skr::ResourceHandle>
{
    inline static void read(ArchiveRead& r, skr::ResourceHandle& v)
    {
        SkrZoneScopedN("Serialize<skr::ResourceHandle>::read");
        GUID resource_id;
        SKR_FAST_CHECK(r.value<GUID>(resource_id), )
        v.set_guid(resource_id);
    }
    inline static void write(ArchiveWrite& w, const skr::ResourceHandle& v)
    {
        w.value<GUID>(v.get_serialized());
    }
};
template <class T>
struct Serialize<skr::AsyncResource<T>>
{
    inline static void read(ArchiveRead& r, skr::AsyncResource<T>& v)
    {
        r.value<ResourceHandle>(v);
    }
    inline static void write(ArchiveWrite& w, const skr::AsyncResource<T>& v)
    {
        w.value<ResourceHandle>(v);
    }
};
} // namespace skr