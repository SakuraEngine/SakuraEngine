#pragma once
#include "SkrCore/platform/vfs.h"
#include "SkrRuntime/resource/resource_handle.h"
#include "SkrRuntime/resource/resource_header.hpp"
#include <SkrContainersDef/span.hpp>

SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service)

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
struct ResourceRegistry;
struct ResourceFactory;
struct ResourceSystem;
struct ResourceSystemImpl;

struct SKR_RUNTIME_API ResourceRequest
{
    virtual ~ResourceRequest() = default;

public:
    virtual skr_guid_t GetGuid() const = 0;
    virtual skr::Span<const uint8_t> GetData() const = 0;
    #ifdef SKR_RESOURCE_DEV_MODE
    virtual skr::Span<const uint8_t> GetArtifactsData() const = 0;
    #endif
    virtual skr::Span<const skr_guid_t> GetDependencies() const = 0;

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

struct SKR_RUNTIME_API ResourceRegistry
{
public:
    virtual bool RequestResourceFile(ResourceRequest* request) = 0;
    virtual void CancelRequestFile(ResourceRequest* requst) = 0;

    void FillRequest(ResourceRequest* request, SResourceHeader header, skr_vfs_t* vfs, const char8_t* uri);
};

struct SKR_RUNTIME_API ResourceSystem
{
    friend struct ::SResourceHandle;

public:
    virtual ~ResourceSystem() = default;
    virtual void Initialize(ResourceRegistry* provider, skr::io::IRAMService* ioService) = 0;
    virtual bool IsInitialized() = 0;
    virtual void Shutdown() = 0;
    virtual void Update() = 0;
    virtual bool WaitRequest() = 0;
    virtual void Quit() = 0;

    virtual void LoadResource(SResourceHandle& handle, bool requireInstalled, uint64_t requester, ESkrRequesterType) = 0;
    virtual void UnloadResource(SResourceHandle& handle) = 0;
    virtual void FlushResource(SResourceHandle& handle) = 0;
    virtual ESkrLoadingStatus GetResourceStatus(const skr_guid_t& handle) = 0;

    virtual ResourceFactory* FindFactory(skr_guid_t type) const = 0;
    virtual void RegisterFactory(ResourceFactory* factory) = 0;
    virtual void UnregisterFactory(skr_guid_t type) = 0;

    virtual ResourceRegistry* GetRegistry() const = 0;
    virtual skr::io::IRAMService* GetRAMService() const = 0;

protected:
    virtual SResourceRecord* _GetOrCreateRecord(const skr_guid_t& guid) = 0;
    virtual SResourceRecord* _GetRecord(const skr_guid_t& guid) = 0;
    virtual SResourceRecord* _GetRecord(void* resource) = 0;
    virtual void _DestroyRecord(SResourceRecord* record) = 0;
};
SKR_RUNTIME_API ResourceSystem* GetResourceSystem();
} // namespace skr
#endif