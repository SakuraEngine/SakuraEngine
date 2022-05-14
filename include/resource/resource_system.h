#pragma once

#include "EASTL/fixed_vector.h"
#include "ecs/entities.hpp"
#include "resource/resource_handle.h"
#include "utils/hashmap.hpp"
#include "resource/resource_header.h"
typedef enum ESkrLoadingPhase
{
    SKR_LOADING_PHASE_NONE = -1,

    // Load Stages
    SKR_LOADING_PHASE_REQUEST_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST,
    SKR_LOADING_PHASE_LOAD_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES,
    SKR_LOADING_PHASE_INSTALL_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE,

    // Unload Stages
    SKR_LOADING_PHASE_UNINSTALL_RESOURCE,
    SKR_LOADING_PHASE_UNLOAD_RESOURCE,

    // Special Cases
    SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE,
    SKR_LOADING_PHASE_CANCEL_WAITFOR_INSTALL_RESOURCE,
    SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES,
    // This stage is needed so we can resume correctly when going from load -> unload -> load
    SKR_LOADING_PHASE_CANCEL_RESOURCE_REQUEST,

    SKR_LOADING_PHASE_FINISHED,
} ESkrLoadingPhase;
#if defined(__cplusplus)
    #include "ghc/filesystem.hpp"
namespace skr
{
namespace resource
{
struct SResourceFactory;
struct SResourceSystem;
struct SResourceRequest {
    SResourceSystem* system;
    skr_resource_record_t* resourceRecord;
    SResourceFactory* factory;
    ghc::filesystem::path path;

    eastl::fixed_vector<skr_guid_t, 4> dependencies;
    ESkrLoadingPhase currentPhase;
    bool isLoading;
    bool isHotReload;
    bool requestInstall;

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

    SResourceRegistry* resourceProvider = nullptr;
    eastl::vector<SResourceRequest*> requests;
    dual::entity_registry_t resourceIds;
    skr::flat_hash_map<skr_guid_t, skr_resource_record_t*, skr::guid::hash> resourceRecords;
    skr::flat_hash_map<void*, skr_resource_record_t*> resourceToRecord;
    skr::flat_hash_map<skr_type_id_t, SResourceFactory*, skr::guid::hash> resourceFactories;
};
RUNTIME_API SResourceSystem* GetResourceSystem();
} // namespace resource
} // namespace skr
#endif