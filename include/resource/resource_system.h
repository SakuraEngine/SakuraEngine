#pragma once

#include "EASTL/fixed_vector.h"
#include "phmap.h"
#include "resource/resource_header.h"
typedef enum ESkrLoadingPhase
{
    SKR_LOADING_PHASE_NONE = -1,

    // Load Stages
    SKR_LOADING_PHASE_REQUEST_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST,
    SKR_LOADING_PHASE_LOAD_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES,
    SKR_LOADING_PHASE_INSTALL_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE,

    // Unload Stages
    SKR_LOADING_PHASE_UNINSTALL_RESOURCE,
    SKR_LOADING_PHASE_UNLOAD_RESOURCE,
    SKR_LOADING_PHASE_UNLOAD_FAILED_RESOURCE,

    // Special Cases
    SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES,
    // This stage is needed so we can resume correctly when going from load -> unload -> load
    SKR_LOADING_PHASE_CANCEL_RESOURCE_REQUEST,

    SKR_LOADING_PHASE_COMPLETE,
} ESkrLoadingPhase;
#if defined(__cplusplus)
    #include "ghc/filesystem.hpp"
namespace skr
{
struct SResourceFactory;
namespace resource
{
struct SResourceSystem;
struct SResourceRequst {
    SResourceSystem* system;
    skr_resource_record_t* resourceRecord;
    SResourceFactory* factory;
    ghc::filesystem::path path;

    eastl::fixed_vector<skr_guid_t, 4> dependencies;
    ESkrLoadingPhase currentPhase;
    bool isLoading;
    bool isHotReload;

    void Update();
};
struct SResourceRegistry {
public:
    virtual void RequestResource(SResourceRequst* request) = 0;
    virtual void CancelRequest(SResourceRequst* requst) = 0;
};
struct SResourceSystem {
    void Initialize(SResourceRegistry* provider);
    bool IsInitialized();
    void Shutdown();
    void Update();

    void RegisterResourceFactory(SResourceFactory* factory);
    void UnregisterResourceFactory(SResourceFactory* factory);

    void LoadResource(skr_resource_handle_t& handle, uint64_t requester = 0);
    void UnloadResource(skr_resource_handle_t& handle);

    skr_resource_record_t* _GetOrCreateRecord(const skr_guid_t& guid);
    skr_resource_record_t* _GetRecord(const skr_guid_t& guid);

    SResourceRegistry* resourceProvider;
    phmap::flat_hash_map<skr_guid_t, skr_resource_record_t*> resourceRecords;
    phmap::flat_hash_map<skr_type_id_t, SResourceFactory*> resourceFactories;
};
} // namespace resource
} // namespace skr
#endif