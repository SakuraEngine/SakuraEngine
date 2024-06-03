#pragma once
#include "SkrOS/thread.h"
#include "SkrRT/resource/resource_handle.h"
#include <SkrContainers/vector.hpp>
#include <SkrContainers/deprecated.hpp>
#include <SkrContainers/stl_vector.hpp>

#include "SkrBase/types.h"

typedef struct skr_resource_header_t {
    uint32_t                                    version;
    skr_guid_t                                  guid;
    skr_guid_t                                  type;
    SKR_RUNTIME_API bool                        ReadWithoutDeps(SBinaryReader* archive);
    skr::InlineVector<skr_resource_handle_t, 4> dependencies;
} skr_resource_header_t;

namespace skr::binary
{
template <>
struct SKR_RUNTIME_API ReadTrait<skr_resource_header_t> {
    static bool Read(SBinaryReader* reader, skr_resource_header_t& header);
};

template <>
struct SKR_RUNTIME_API WriteTrait<skr_resource_header_t> {
    static bool Write(SBinaryWriter* writer, const skr_resource_header_t& header);
};
} // namespace skr::binary

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
    SKR_LOADING_STATUS_COUNT
} ESkrLoadingStatus;

typedef struct skr_resource_record_t skr_resource_record_t;
struct lua_State;
#define TRACK_RESOURCE_REQUESTS 1
namespace skr::resource
{
struct SResourceRequest;
}
struct SKR_RUNTIME_API skr_resource_record_t {
    void* resource             = nullptr;
    void  (*destructor)(void*) = nullptr;
#ifdef SKR_RESOURCE_DEV_MODE
    void* artifacts                     = nullptr;
    void  (*artifactsDestructor)(void*) = nullptr;
#endif
    struct callback_t {
        void* data;
        void  (*callback)(void*);
        void  operator()(void) const { callback(data); }
    };
    skr::stl_vector<callback_t> callbacks[SKR_LOADING_STATUS_COUNT];
    SMutexObject              mutex;
    ESkrLoadingStatus         loadingStatus = SKR_LOADING_STATUS_UNLOADED;
#ifdef TRACK_RESOURCE_REQUESTS
    struct object_requester {
        uint32_t          id;
        void*             requester = nullptr;
        ESkrRequesterType type;
        bool              operator==(const object_requester& other) const { return id == other.id && type == other.type; };
    };
    struct entity_requester {
        uint32_t               id;
        struct sugoi_storage_t* storage        = nullptr;
        uint32_t               entityRefCount = 0;
        bool                   operator==(const entity_requester& other) const { return id == other.id; };
    };
    struct script_requester {
        uint32_t          id;
        struct lua_State* state          = nullptr;
        uint32_t          scriptRefCount = 0;
        bool              operator==(const entity_requester& other) const { return id == other.id; };
    };
    uint32_t                        id               = 0;
    uint32_t                        requesterCounter = 0;
    skr::stl_vector<object_requester> objectReferences;
    skr::stl_vector<entity_requester> entityReferences;
    skr::stl_vector<script_requester> scriptReferences;
    uint32_t                        entityRefCount = 0;
    uint32_t                        scriptRefCount = 0;
#else
    std::atomic<uint32_t> referenceCount = 0;
#endif
    skr_resource_header_t            header;
    skr::resource::SResourceRequest* activeRequest;

    void SetStatus(ESkrLoadingStatus);
    void AddCallback(ESkrLoadingStatus, void (*callback)(void*), void* userData);
    template <class F>
    void AddCallback(ESkrLoadingStatus status, const F& callback)
    {
        AddCallback(
        status, [](void* data) { (*static_cast<F*>(data))(); }, (void*)&callback);
    }
    void     SetResource(void* resource, void (*destructor)(void*));
    uint32_t AddReference(uint64_t requester, ESkrRequesterType requesterType);
    void     RemoveReference(uint32_t id, ESkrRequesterType requesterType);
    bool     IsReferenced() const;
};