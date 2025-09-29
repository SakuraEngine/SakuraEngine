#pragma once
#include "SkrBase/config/key_words.h"
#include <SkrBase/type_info.hpp>
#include <SkrCore/serialize/serialize_traits.hpp>

enum class EResourceLoadingStatus : uint32_t;
struct SResourceRecord;
namespace skr { struct ResourceSystemImpl; }

struct SKR_RUNTIME_API SResourceHandle
{
public:
    SResourceHandle();
    SResourceHandle(const SResourceHandle& other);
    SResourceHandle& operator=(const SResourceHandle& other);
    SResourceHandle(const skr::GUID& other);
    SResourceHandle& operator=(const skr::GUID& other);
    SResourceHandle(SResourceHandle&& other);
    SResourceHandle& operator=(SResourceHandle&& other);
    ~SResourceHandle();
    operator bool() const;

    void* load();
    void* install();
    void reset();

    void* get_loaded() const;
    void* get_installed() const;

    bool is_null() const;
    bool is_guid() const;
    bool is_loaded() const;
    bool is_installed() const;

    skr::GUID get_guid() const;
    skr::GUID get_type() const;
    
    // if resolve is false, then unresolve handle will always return EResourceLoadingStatus::Unloaded
    EResourceLoadingStatus get_status() const;

    SResourceHandle clone();
    SResourceHandle clone(struct sugoi_storage_t* requester);

protected:
    void* load(bool requireInstalled);
    SResourceRecord* get_record() const;
    void acquire_record(SResourceRecord* record) const; 

    friend struct skr::Serialize<SResourceHandle>;
    friend struct skr::ResourceSystemImpl;
    union
    {
        skr::GUID guid;
        struct
        {
            mutable uint64_t padding;     // zero, flag for resolved or not
            // since resource record is allocated with alignment 8, the lower 3 bits should always be zero
            // so we put requester type into it
            mutable uint64_t pointer; // resource record ptr & requester type
        };
    };
};
SKR_TYPE_INFO(SResourceHandle, "A9E0CE3D-5E9B-45F1-AC28-B882885C63AB");

SKR_EXTERN_C SKR_RUNTIME_API int skr_is_resource_resolved(SResourceHandle* handle);
SKR_EXTERN_C SKR_RUNTIME_API void skr_get_resource_guid(SResourceHandle* handle, skr::GUID* guid);
SKR_EXTERN_C SKR_RUNTIME_API void skr_get_resource(SResourceHandle* handle, void** guid);

namespace skr
{
using ResourceHandle = SResourceHandle;
template <class T>
struct AsyncResource : public ResourceHandle
{
public:
    AsyncResource() SKR_NOEXCEPT;
    AsyncResource(const skr::GUID& guid) SKR_NOEXCEPT;
    AsyncResource(const ResourceHandle& guid) SKR_NOEXCEPT;
    AsyncResource(const AsyncResource& guid) SKR_NOEXCEPT;

    inline T* load() { return (T*)ResourceHandle::load(); }
    inline T* install() { return (T*)ResourceHandle::install(); }

    inline T* get_loaded() const { return (T*)ResourceHandle::get_loaded(); }
    inline T* get_installed() const { return (T*)ResourceHandle::get_installed(); }

    inline AsyncResource clone() { return *this; }
    inline AsyncResource clone(struct sugoi_storage_t* requester) { return *this; }
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

template <class T>
inline AsyncResource<T>::AsyncResource(const ResourceHandle& hdl) SKR_NOEXCEPT
    : ResourceHandle(hdl)
{
}

template <class T>
inline AsyncResource<T>::AsyncResource(const AsyncResource& hdl) SKR_NOEXCEPT
    : ResourceHandle(hdl)
{
}

template <>
struct Serialize<skr::ResourceHandle>
{
    inline static void read(ArchiveRead& r, skr::ResourceHandle& v)
    {
        SkrZoneScopedN("Serialize<skr::ResourceHandle>::read");
        GUID resource_id;
        SKR_FAST_CHECK(r.value<GUID>(resource_id), )
        v.guid = (resource_id);
    }
    inline static void write(ArchiveWrite& w, const skr::ResourceHandle& v)
    {
        w.value<GUID>(v.get_guid());
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