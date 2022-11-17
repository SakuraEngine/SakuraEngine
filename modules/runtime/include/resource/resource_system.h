#pragma once
#include <EASTL/fixed_vector.h>
#include "ecs/entities.hpp"
#include "platform/thread.h"
#include "platform/guid.hpp"
#include "platform/vfs.h"
#include "resource/resource_handle.h"
#include "containers/hashmap.hpp"
#include "resource/resource_header.h"
#include "utils/io.h"
#include "utils/types.h"

namespace skr::io { class RAMService; }

typedef enum ESkrLoadingPhase
{
    SKR_LOADING_PHASE_NONE = -1,

    // Load Stages
    SKR_LOADING_PHASE_REQUEST_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST,
    SKR_LOADING_PHASE_IO,
    SKR_LOADING_PHASE_WAITFOR_IO,
    SKR_LOADING_PHASE_LOAD_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES,
    SKR_LOADING_PHASE_INSTALL_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE,

    // Unload Stages
    SKR_LOADING_PHASE_UNINSTALL_RESOURCE,
    SKR_LOADING_PHASE_UNLOAD_RESOURCE,

    // Special Cases, needed so we can resume correctly when going from load -> unload -> load
    SKR_LOADING_PHASE_CANCLE_WAITFOR_IO,
    SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE,
    SKR_LOADING_PHASE_CANCEL_WAITFOR_INSTALL_RESOURCE,
    SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES,
    SKR_LOADING_PHASE_CANCEL_RESOURCE_REQUEST,

    SKR_LOADING_PHASE_FINISHED,
    SKR_LOADING_PHASE_FAILED,
} ESkrLoadingPhase;

#if defined(__cplusplus)
#include <platform/filesystem.hpp>

namespace skr
{
namespace resource
{
struct SResourceRegistry;
struct SResourceFactory;
struct SResourceSystem;

struct RUNTIME_API SResourceRequest {
    friend struct SResourceSystem;
    friend struct SResourceRegistry;
public:
    skr_guid_t GetGuid() const;
    gsl::span<const uint8_t> GetData() const;
    gsl::span<const skr_guid_t> GetDependencies() const;

    void UpdateLoad(bool requestInstall);
    void UpdateUnload();
    void Update();

    bool Okay();
    bool Yielded();
    bool Failed();

    void OnRequestFileFinished();
    void OnRequestLoadFinished();
protected:
    void _LoadFinished();
    void _InstallFinished();
    void _UnloadResource();

    ESkrLoadingPhase currentPhase;
    std::atomic_bool isLoading;
    std::atomic_bool requireLoading;
    std::atomic_bool requestInstall;

    SResourceSystem* system;
    SResourceFactory* factory;
    skr_vfs_t* vfs;

    eastl::fixed_vector<skr_guid_t, 4> dependencies;
    skr_resource_record_t* resourceRecord;
    skr_async_request_t ioRequest;
    skr_async_ram_destination_t ioDestination;
    eastl::string resourceUrl;
    uint8_t* data;
    uint64_t size;
};

struct RUNTIME_API SResourceRegistry {
public:
    virtual bool RequestResourceFile(SResourceRequest* request) = 0;
    virtual void CancelRequestFile(SResourceRequest* requst) = 0;

    void FillRequest(SResourceRequest* request, skr_resource_header_t header, skr_vfs_t* vfs, const char* uri)
    {
        if (request)
        {
            request->resourceRecord->header.type = header.type;
            request->resourceRecord->header.version = header.version;
            request->vfs = vfs;
            request->resourceUrl = uri;
        }
    }
};

struct RUNTIME_API SResourceSystem {
    friend struct ::skr_resource_handle_t;
public:
    SResourceSystem();
    ~SResourceSystem();
    void Initialize(SResourceRegistry* provider, skr::io::RAMService* ioService);
    bool IsInitialized();
    void Shutdown();
    void Update();

    void LoadResource(skr_resource_handle_t& handle,
        bool requireInstalled, uint64_t requester, ESkrRequesterType);
    void UnloadResource(skr_resource_handle_t& handle);
    ESkrLoadingStatus GetResourceStatus(const skr_guid_t& handle);

    SResourceFactory* FindFactory(skr_type_id_t type) const;
    void RegisterFactory(skr_type_id_t type, SResourceFactory* factory);
    void UnregisterFactory(skr_type_id_t type);

    SResourceRegistry* GetRegistry() const;
    skr::io::RAMService* GetRAMService() const;

protected:
    skr_resource_record_t* _GetOrCreateRecord(const skr_guid_t& guid);
    skr_resource_record_t* _GetRecord(const skr_guid_t& guid);
    skr_resource_record_t* _GetRecord(void* resource);
    void _DestroyRecord(const skr_guid_t& guid, skr_resource_record_t* record);

    SResourceRegistry* resourceRegistry = nullptr;
    skr::io::RAMService* ioService = nullptr; 
    eastl::vector<SResourceRequest*> requests;
    dual::entity_registry_t resourceIds;
    SMutex recordMutex;
    skr::flat_hash_map<skr_guid_t, skr_resource_record_t*, skr::guid::hash> resourceRecords;
    skr::flat_hash_map<void*, skr_resource_record_t*> resourceToRecord;
    skr::flat_hash_map<skr_type_id_t, SResourceFactory*, skr::guid::hash> resourceFactories;
};
RUNTIME_API SResourceSystem* GetResourceSystem();
} // namespace resource
} // namespace skr
#endif