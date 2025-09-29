#pragma once
#include "SkrCore/platform/vfs.h"
#include "SkrRuntime/resource/resource_handle.h"
#include "SkrRuntime/resource/resource_header.hpp"
#include <SkrContainersDef/span.hpp>

SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service)

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
    virtual skr::GUID GetGuid() const = 0;
    virtual skr::Span<const uint8_t> GetData() const = 0;
    virtual skr::Span<const SResourceHandle> GetDependencies() const = 0;

    virtual void Update() = 0;
    virtual bool Okay() = 0;
    virtual bool Failed() = 0;
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

    virtual SResourceRecord* LoadResource(const SResourceHandle& handle, bool requireInstalled) = 0;
    virtual SResourceRecord* FindResourceRecord(const skr::GUID& guid) = 0;
    virtual SResourceHandle EnqueueResource(skr::GUID guid, skr::GUID type, void* resource, bool requireInstalled, skr::Vector<SResourceHandle> dependencies = {}, EResourceLoadingStatus status = EResourceLoadingStatus::Loaded) = 0;
    virtual void UnloadResource(const SResourceHandle& handle) = 0;
    virtual void FlushResource(const SResourceHandle& handle) = 0;

    virtual ResourceFactory* FindFactory(skr::GUID type) const = 0;
    virtual void RegisterFactory(ResourceFactory* factory) = 0;
    virtual void UnregisterFactory(skr::GUID type) = 0;

    virtual ResourceRegistry* GetRegistry() const = 0;
    virtual skr::io::IRAMService* GetRAMService() const = 0;

    template <typename T>
    inline AsyncResource<T> EnqueueResource(skr::GUID guid, T* resource, bool requireInstalled, skr::Vector<SResourceHandle> dependencies = {}, EResourceLoadingStatus status = EResourceLoadingStatus::Loaded)
    {
        return (AsyncResource<T>)EnqueueResource(guid, skr::type_id_of<T>(), resource, requireInstalled, dependencies, status);
    }

protected:
    virtual SResourceRecord* _GetOrCreateRecord(const skr::GUID& guid) = 0;
    virtual SResourceRecord* _GetRecord(void* resource) = 0;
    virtual void _DestroyRecord(SResourceRecord* record) = 0;
};
SKR_RUNTIME_API ResourceSystem* GetResourceSystem();
} // namespace skr