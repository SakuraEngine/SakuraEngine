#pragma once
#include <EASTL/fixed_vector.h>
#include "ecs/entities.hpp"
#include "platform/thread.h"
#include "platform/guid.hpp"
#include "platform/vfs.h"
#include "resource/resource_handle.h"
#include "utils/hashmap.hpp"
#include "resource/resource_header.h"
#include "utils/io.h"
#include "utils/types.h"

namespace skr::io
{
class RAMService;
}

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
} ESkrLoadingPhase;
#if defined(__cplusplus)
    #include <platform/filesystem.hpp>
namespace skr
{
namespace resource
{
struct SResourceFactory;
struct SResourceSystem;
struct SResourceRequest {
    SResourceSystem* system;
    skr_resource_record_t* resourceRecord;
    skr_async_io_request_t request;
    skr_async_ram_destination_t destination;
    SResourceFactory* factory;
    skr_vfs_t* vfs;
    skr::filesystem::path path;
    std::string u8path;
    uint8_t* data;
    uint64_t size;

    gsl::span<uint8_t> GetData() { return {data, data+size}; }

    eastl::fixed_vector<skr_guid_t, 4> dependencies;
    ESkrLoadingPhase currentPhase;
    bool isLoading;
    std::atomic_bool requireLoading;
    std::atomic_bool requestInstall;

    void UpdateLoad(bool requestInstall);
    void UpdateUnload();
    void Update();
    bool Yielded();
    void OnRequestFileFinished();
    void OnRequestLoadFinished();
    void _LoadFinished();
    void _InstallFinished();
    void _UnloadResource();
};
struct RUNTIME_API SResourceRegistry {
public:
    virtual void RequestResourceFile(SResourceRequest* request) = 0;
    virtual void CancelRequestFile(SResourceRequest* requst) = 0;
};
struct RUNTIME_API SResourceSystem {
    SResourceSystem();
    ~SResourceSystem();
    void Initialize(SResourceRegistry* provider);
    bool IsInitialized();
    void Shutdown();
    void Update();

    void LoadResource(skr_resource_handle_t& handle, bool requireInstalled = true, uint32_t requester = 0, ESkrRequesterType = SKR_REQUESTER_UNKNOWN);
    void UnloadResource(skr_resource_handle_t& handle);

    skr_resource_record_t* _GetOrCreateRecord(const skr_guid_t& guid);
    skr_resource_record_t* _GetRecord(const skr_guid_t& guid);
    skr_resource_record_t* _GetRecord(void* resource);
    void _DestroyRecord(const skr_guid_t& guid, skr_resource_record_t* record);

    void RegisterFactory(skr_type_id_t type, SResourceFactory* factory);
    void UnregisterFactory(skr_type_id_t type);

    SResourceRegistry* resourceProvider = nullptr;
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