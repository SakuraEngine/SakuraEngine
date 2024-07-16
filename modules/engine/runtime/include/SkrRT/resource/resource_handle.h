#pragma once
#include "SkrBase/types.h"
#include "SkrRTTR/rttr_traits.hpp"

enum ESkrLoadingStatus : uint32_t;
struct skr_resource_record_t;
enum ESkrRequesterType
{
    SKR_REQUESTER_ENTITY     = 0,
    SKR_REQUESTER_DEPENDENCY = 1,
    SKR_REQUESTER_SYSTEM     = 2,
    SKR_REQUESTER_SCRIPT     = 3,
    SKR_REQUESTER_UNKNOWN    = 4
};
struct lua_State;
typedef struct skr_resource_handle_t {
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
#if defined(__cplusplus)
    SKR_RUNTIME_API skr_resource_handle_t();
    SKR_RUNTIME_API ~skr_resource_handle_t();
    SKR_RUNTIME_API                        skr_resource_handle_t(const skr_guid_t& other);
    SKR_RUNTIME_API                        skr_resource_handle_t(const skr_resource_handle_t& other);
    SKR_RUNTIME_API                        skr_resource_handle_t(const skr_resource_handle_t& other, uint64_t requester, ESkrRequesterType requesterType);
    SKR_RUNTIME_API                        skr_resource_handle_t(skr_resource_handle_t&& other);
    SKR_RUNTIME_API skr_resource_handle_t& operator=(const skr_resource_handle_t& other);
    SKR_RUNTIME_API skr_resource_handle_t& operator=(const skr_guid_t& other);
    SKR_RUNTIME_API skr_resource_handle_t& operator=(skr_resource_handle_t&& other);
    SKR_RUNTIME_API void                   set_ptr(void* ptr);
    SKR_RUNTIME_API void                   set_guid(const skr_guid_t& guid);
    SKR_RUNTIME_API bool                   is_resolved() const;
    SKR_RUNTIME_API void*                  get_resolved(bool requireInstalled = true) const;
    SKR_RUNTIME_API skr_guid_t             get_serialized() const;
    SKR_RUNTIME_API void                   resolve(bool requireInstalled, uint64_t requester, ESkrRequesterType requesterType);
    void                                   resolve(bool requireInstalled, struct sugoi_storage_t* requester)
    {
        resolve(requireInstalled, (uint64_t)requester, SKR_REQUESTER_ENTITY);
    }
    SKR_RUNTIME_API void       unload();
    SKR_RUNTIME_API skr_guid_t get_guid() const;
    SKR_RUNTIME_API skr_guid_t get_type() const;
    SKR_RUNTIME_API void*      get_ptr() const;
    SKR_RUNTIME_API bool       is_null() const;
    SKR_RUNTIME_API void       reset();
    skr_resource_handle_t      clone(uint64_t requester, ESkrRequesterType requesterType)
    {
        return { *this, requester, requesterType };
    }
    skr_resource_handle_t clone(struct sugoi_storage_t* requester)
    {
        return { *this, (uint64_t)requester, SKR_REQUESTER_ENTITY };
    }
    SKR_RUNTIME_API uint32_t          get_requester_id() const;
    SKR_RUNTIME_API ESkrRequesterType get_requester_type() const;
    // if resolve is false, then unresolve handle will always return SKR_LOADING_STATUS_UNLOADED
    SKR_RUNTIME_API ESkrLoadingStatus      get_status(bool resolve = false) const;
    SKR_RUNTIME_API skr_resource_record_t* get_record() const;
    SKR_RUNTIME_API void                   set_record(skr_resource_record_t* record);
    SKR_RUNTIME_API void                   set_resolved(skr_resource_record_t* record, uint32_t requesterId, ESkrRequesterType requesterType);
#endif
} skr_resource_handle_t;
SKR_RTTR_TYPE(skr_resource_handle_t, "A9E0CE3D-5E9B-45F1-AC28-B882885C63AB");

#if defined(__cplusplus)
namespace skr::resource
{
template <class T>
struct TResourceHandle : skr_resource_handle_t {
    using skr_resource_handle_t::skr_resource_handle_t;
    // TODO: T* resolve
    T* get_resolved(bool requireInstalled = true) const
    {
        return (T*)skr_resource_handle_t::get_resolved(requireInstalled);
    }
    T* get_ptr() const
    {
        return (T*)skr_resource_handle_t::get_ptr();
    }
    TResourceHandle clone(uint64_t requester, ESkrRequesterType requesterType)
    {
        return { *this, requester, requesterType };
    }
    TResourceHandle clone(struct sugoi_storage_t* requester)
    {
        return { *this, (uint64_t)requester, SKR_REQUESTER_ENTITY };
    }
};
} // namespace skr::resource
#endif

#ifdef __cplusplus
    #define SKR_RESOURCE_HANDLE(type) skr::resource::TResourceHandle<type>
    #define SKR_RESOURCE_FIELD(type, name) skr::resource::TResourceHandle<type> name
#else
    #define SKR_RESOURCE_HANDLE(type) skr_resource_handle_t
    #define SKR_RESOURCE_FIELD(type, name) skr_resource_handle_t name
#endif

SKR_RUNTIME_API int  skr_is_resource_resolved(skr_resource_handle_t* handle);
SKR_RUNTIME_API void skr_get_resource_guid(skr_resource_handle_t* handle, skr_guid_t* guid);
SKR_RUNTIME_API void skr_get_resource(skr_resource_handle_t* handle, void** guid);

// bin serde
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <>
struct BinSerde<skr_resource_handle_t> {
    inline static bool read(SBinaryReader* r, skr_resource_handle_t& v)
    {
        skr_guid_t guid;
        if (!bin_read(r, guid))
        {
            SKR_LOG_FATAL(u8"failed to read resource handle guid! ret code: %d", -1);
            return false;
        }
        v.set_guid(guid);
        return true;
    }
    inline static bool write(SBinaryWriter* w, const skr_resource_handle_t& v)
    {
        return bin_write(w, v.get_serialized());
    }
};

template <class T>
struct BinSerde<skr::resource::TResourceHandle<T>> {
    inline static bool read(SBinaryReader* archive, skr::resource::TResourceHandle<T>& handle)
    {
        skr_guid_t guid;
        if (!bin_read(archive, (guid))) return false;
        handle.set_guid(guid);
        return true;
    }
    inline static bool write(SBinaryWriter* binary, const skr::resource::TResourceHandle<T>& handle)
    {
        const auto& hdl = static_cast<const skr_resource_handle_t&>(handle);
        return bin_write(binary, hdl);
    }
};
} // namespace skr

// json serde
#include "SkrSerde/json_serde.hpp"
namespace skr
{
template <>
struct JsonSerde<skr_resource_handle_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_resource_handle_t& v)
    {
        SkrZoneScopedN("JsonSerde<skr_resource_handle_t>::read");
        skr::String view;
        SKR_EXPECTED_CHECK(r->String(view), false);
        {
            skr_guid_t guid;
            if (!skr::guid_from_sv(view.u8_str(), guid))
                return false;
            v.set_guid(guid);
        }
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_resource_handle_t& v)
    {
        return json_write<skr_guid_t>(w, v.get_serialized());
    }
};
template <class T>
struct JsonSerde<skr::resource::TResourceHandle<T>> {
    inline static bool read(skr::archive::JsonReader* r, skr::resource::TResourceHandle<T>& v)
    {
        return json_read<skr_resource_handle_t>(r, (skr_resource_handle_t&)v);
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr::resource::TResourceHandle<T>& v)
    {
        return json_write<skr_resource_handle_t>(w, (const skr_resource_handle_t&)v);
    }
};
} // namespace skr