#pragma once
#include <EASTL/fixed_vector.h>
#include "ecs/entities.hpp"
#include "platform/thread.h"
#include "platform/guid.hpp"
#include "platform/vfs.h"
#include "resource/resource_handle.h"
#include "resource/resource_header.hpp"
#include "utils/io.h"
#include "utils/types.h"
#include "task/task.hpp"

namespace skr::io { class RAMService; }

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
#include <platform/filesystem.hpp>

namespace skr
{
namespace resource
{
struct SResourceRegistry;
struct SResourceFactory;
struct SResourceSystem;
struct SResourceSystemImpl;

struct RUNTIME_API SResourceRequest {
    friend struct SResourceSystem;
    friend struct SResourceRegistry;
    friend struct SResourceSystemImpl;
public:
    skr_guid_t GetGuid() const;
    gsl::span<const uint8_t> GetData() const;
#ifdef SKR_RESOURCE_DEV_MODE
    gsl::span<const uint8_t> GetArtifactsData() const;
#endif
    gsl::span<const skr_guid_t> GetDependencies() const;

    void UpdateLoad(bool requestInstall);
    void UpdateUnload();
    void Update();

    bool Okay();
    bool Yielded();
    bool Failed();
    bool AsyncSerde();

    void OnRequestFileFinished();
    void OnRequestLoadFinished();

    void LoadTask();
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
    skr::string resourceUrl;
#ifdef SKR_RESOURCE_DEV_MODE
    skr_async_request_t artifactsIoRequest;
    skr_async_ram_destination_t artifactsIoDestination;
    skr::string artifactsUrl;
#endif
    uint8_t* data;
    uint64_t size;
#ifdef SKR_RESOURCE_DEV_MODE
    uint8_t* artifactsData;
    uint64_t artifactsSize;
#endif
    skr::task::event_t serdeEvent;
    bool serdeScheduled;
    int serdeResult; 
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
    virtual ~SResourceSystem() = default;
    virtual void Initialize(SResourceRegistry* provider, skr::io::RAMService* ioService) = 0;
    virtual bool IsInitialized() = 0;
    virtual void Shutdown() = 0;
    virtual void Update() = 0;

    virtual void LoadResource(skr_resource_handle_t& handle, bool requireInstalled, uint64_t requester, ESkrRequesterType) = 0;
    virtual void UnloadResource(skr_resource_handle_t& handle) = 0;
    virtual void FlushResource(skr_resource_handle_t& handle) = 0;
    virtual ESkrLoadingStatus GetResourceStatus(const skr_guid_t& handle) = 0;

    virtual SResourceFactory* FindFactory(skr_type_id_t type) const = 0;
    virtual void RegisterFactory(SResourceFactory* factory) = 0;
    virtual void UnregisterFactory(skr_type_id_t type) = 0;

    virtual SResourceRegistry* GetRegistry() const = 0;
    virtual skr::io::RAMService* GetRAMService() const = 0;

protected:
    virtual skr_resource_record_t* _GetOrCreateRecord(const skr_guid_t& guid) = 0;
    virtual skr_resource_record_t* _GetRecord(const skr_guid_t& guid) = 0;
    virtual skr_resource_record_t* _GetRecord(void* resource) = 0;
    virtual void _DestroyRecord(const skr_guid_t& guid, skr_resource_record_t* record) = 0;
};
RUNTIME_API SResourceSystem* GetResourceSystem();
} // namespace resource
} // namespace skr
#endif