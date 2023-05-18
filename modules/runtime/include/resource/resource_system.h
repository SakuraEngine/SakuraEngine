#pragma once
#include "platform/vfs.h"
#include "resource/resource_handle.h"
#include "resource/resource_header.hpp"
#include "utils/types.h"

struct skr_io_ram_service_t;

typedef enum ESkrLoadingPhase
{
    SKR_LOADING_PHASE_NONE = -1,

    // Load Stages
    SKR_LOADING_PHASE_REQUEST_RESOURCE,
    SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST,
    SKR_LOADING_PHASE_IO,
    SKR_LOADING_PHASE_WAITFOR_IO,
    SKR_LOADING_PHASE_DESER_RESOURCE,
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

namespace skr
{
namespace resource
{
struct SResourceRegistry;
struct SResourceFactory;
struct SResourceSystem;
struct SResourceSystemImpl;

struct RUNTIME_API SResourceRequest {
    virtual ~SResourceRequest() = default;
public:
    virtual skr_guid_t GetGuid() const = 0;
    virtual skr::span<const uint8_t> GetData() const = 0;
#ifdef SKR_RESOURCE_DEV_MODE
    virtual skr::span<const uint8_t> GetArtifactsData() const = 0;
#endif
    virtual skr::span<const skr_guid_t> GetDependencies() const = 0;

    virtual void UpdateLoad(bool requestInstall) = 0;
    virtual void UpdateUnload() = 0;
    virtual void Update() = 0;

    virtual bool Okay() = 0;
    virtual bool Yielded() = 0;
    virtual bool Failed() = 0;
    virtual bool AsyncSerde() = 0;

    virtual void OnRequestFileFinished() = 0;
    virtual void OnRequestLoadFinished() = 0;

    virtual void LoadTask() = 0;
protected:
    virtual void _LoadDependencies() = 0;
    virtual void _UnloadDependencies() = 0;
    virtual void _LoadFinished() = 0;
    virtual void _InstallFinished() = 0;
    virtual void _UnloadResource() = 0;
};

struct RUNTIME_API SResourceRegistry {
public:
    virtual bool RequestResourceFile(SResourceRequest* request) = 0;
    virtual void CancelRequestFile(SResourceRequest* requst) = 0;

    void FillRequest(SResourceRequest* request, skr_resource_header_t header, skr_vfs_t* vfs, const char8_t* uri);
};

struct RUNTIME_API SResourceSystem {
    friend struct ::skr_resource_handle_t;
public:
    virtual ~SResourceSystem() = default;
    virtual void Initialize(SResourceRegistry* provider, skr_io_ram_service_t* ioService) = 0;
    virtual bool IsInitialized() = 0;
    virtual void Shutdown() = 0;
    virtual void Update() = 0;
    virtual bool WaitRequest() = 0;
    virtual void Quit() = 0;

    virtual void LoadResource(skr_resource_handle_t& handle, bool requireInstalled, uint64_t requester, ESkrRequesterType) = 0;
    virtual void UnloadResource(skr_resource_handle_t& handle) = 0;
    virtual void FlushResource(skr_resource_handle_t& handle) = 0;
    virtual ESkrLoadingStatus GetResourceStatus(const skr_guid_t& handle) = 0;

    virtual SResourceFactory* FindFactory(skr_type_id_t type) const = 0;
    virtual void RegisterFactory(SResourceFactory* factory) = 0;
    virtual void UnregisterFactory(skr_type_id_t type) = 0;

    virtual SResourceRegistry* GetRegistry() const = 0;
    virtual skr_io_ram_service_t* GetRAMService() const = 0;

protected:
    virtual skr_resource_record_t* _GetOrCreateRecord(const skr_guid_t& guid) = 0;
    virtual skr_resource_record_t* _GetRecord(const skr_guid_t& guid) = 0;
    virtual skr_resource_record_t* _GetRecord(void* resource) = 0;
    virtual void _DestroyRecord(skr_resource_record_t* record) = 0;
};
RUNTIME_API SResourceSystem* GetResourceSystem();
} // namespace resource
} // namespace skr
#endif