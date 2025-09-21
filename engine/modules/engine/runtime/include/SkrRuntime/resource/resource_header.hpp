#pragma once
#include "SkrOS/thread.h"
#include "SkrContainersDef/vector.hpp"
#include "SkrRuntime/resource/resource_handle.h"

namespace skr
{
struct ResourceRequest;
}
typedef struct SResourceRecord SResourceRecord;
#define TRACK_RESOURCE_REQUESTS 1

typedef struct SResourceHeader
{
    uint32_t version;
    skr_guid_t guid;
    skr_guid_t type;
    skr::InlineVector<SResourceHandle, 4> dependencies;
} SResourceHeader;

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

struct SKR_RUNTIME_API SResourceRecord
{
    void* resource = nullptr;
    void (*destructor)(void*) = nullptr;
#ifdef SKR_RESOURCE_DEV_MODE
    void* artifacts = nullptr;
    void (*artifactsDestructor)(void*) = nullptr;
#endif
    struct callback_t
    {
        void* data;
        void (*callback)(void*);
        void operator()(void) const { callback(data); }
    };
    skr::Vector<callback_t> callbacks[SKR_LOADING_STATUS_COUNT];
    SMutexObject mutex;
    ESkrLoadingStatus loadingStatus = SKR_LOADING_STATUS_UNLOADED;
#ifdef TRACK_RESOURCE_REQUESTS
    struct object_requester
    {
        uint32_t id;
        void* requester = nullptr;
        ESkrRequesterType type;
        bool operator==(const object_requester& other) const { return id == other.id && type == other.type; };
    };
    struct entity_requester
    {
        uint32_t id;
        struct sugoi_storage_t* storage = nullptr;
        uint32_t entityRefCount = 0;
        bool operator==(const entity_requester& other) const { return id == other.id; };
    };
    struct script_requester
    {
        uint32_t id;
        struct lua_State* state = nullptr;
        uint32_t scriptRefCount = 0;
        bool operator==(const entity_requester& other) const { return id == other.id; };
    };
    uint32_t id = 0;
    uint32_t requesterCounter = 0;
    skr::Vector<object_requester> objectReferences;
    skr::Vector<entity_requester> entityReferences;
    skr::Vector<script_requester> scriptReferences;
    uint32_t entityRefCount = 0;
    uint32_t scriptRefCount = 0;
#else
    std::atomic<uint32_t> referenceCount = 0;
#endif
    SResourceHeader header;
    skr::ResourceRequest* activeRequest;

    void SetStatus(ESkrLoadingStatus);
    void AddCallback(ESkrLoadingStatus, void (*callback)(void*), void* userData);
    template <class F>
    void AddCallback(ESkrLoadingStatus status, const F& callback)
    {
        AddCallback(
            status, [](void* data) { (*static_cast<F*>(data))(); }, (void*)&callback
        );
    }
    void SetResource(void* resource, void (*destructor)(void*));
    uint32_t AddReference(uint64_t requester, ESkrRequesterType requesterType);
    void RemoveReference(uint32_t id, ESkrRequesterType requesterType);
    bool IsReferenced() const;
};

namespace skr
{
template <>
struct SKR_RUNTIME_API Serialize<SResourceHeader>
{
    static void read(ArchiveRead& r, SResourceHeader& v);
    static void write(ArchiveWrite& w, const SResourceHeader& v);
};
}; // namespace skr