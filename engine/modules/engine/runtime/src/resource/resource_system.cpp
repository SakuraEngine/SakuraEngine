#include "resource_request_impl.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrContainersDef/stl_vector.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrRuntime/resource/resource_factory.hpp"
#include "SkrContainersDef/concurrent_queue.hpp"

#include "SkrRuntime/sugoi/entity_registry.hpp"

namespace skr
{
struct SKR_RUNTIME_API ResourceSystemImpl : public ResourceSystem
{
    friend struct ::SResourceHandle;

public:
    ResourceSystemImpl();
    ~ResourceSystemImpl();
    void Initialize(ResourceRegistry* provider, skr::io::IRAMService* ioService) final override;
    bool IsInitialized() final override;
    void Shutdown() final override;
    void Update() final override;
    bool WaitRequest() final override;
    void Quit() final override;

    SResourceRecord* LoadResource(const SResourceHandle& handle, bool requireInstalled) final override;
    SResourceRecord* FindResourceRecord(const GUID& guid) final override;
    SResourceHandle EnqueueResource(skr::GUID guid, skr::GUID type, void* resource, bool requireInstalled, skr::Vector<SResourceHandle> dependencies, EResourceLoadingStatus status) final override;
    void UnloadResource(const SResourceHandle& handle) final override;
    void FlushResource(const SResourceHandle& handle) final override;

    ResourceFactory* FindFactory(GUID type) const final override;
    void RegisterFactory(ResourceFactory* factory) final override;
    void UnregisterFactory(GUID type) final override;

    ResourceRegistry* GetRegistry() const final override;
    skr::io::IRAMService* GetRAMService() const final override;

protected:
    SResourceRecord* _GetOrCreateRecord(const GUID& guid) final override;
    SResourceRecord* _GetRecord(void* resource) final override;
    void _UnloadResource(SResourceRecord* record);
    void _DestroyRecord(SResourceRecord* record) final override;
    void _ClearFinishedRequests();

    ResourceRegistry* resourceRegistry = nullptr;
    skr::io::IRAMService* ioService = nullptr;

    struct ResourceRequestConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
    {
        static constexpr const char* kResourceRequestQueueName = "ResourceRequestQueue";
        static const bool RECYCLE_ALLOCATED_BLOCKS = true;
        static inline void* malloc(size_t size) { return sakura_mallocN(size, kResourceRequestQueueName); }
        static inline void free(void* ptr) { return sakura_freeN(ptr, kResourceRequestQueueName); }
    };

    skr::ConcurrentQueue<ResourceRequest*, ResourceRequestConcurrentQueueTraits> requests;
    SMutexObject recordMutex; // this mutex is used to protect the resourceRecords and resourceToRecord maps

    // these requests are only handled inside this system and is thread-unsafe

    skr::stl_vector<ResourceRequest*> failedRequests;
    skr::stl_vector<ResourceRequest*> toUpdateRequests;
    skr::stl_vector<ResourceRequest*> serdeBatch;

    task::counter_t counter;
    bool quit = false;
    skr::ParallelFlatHashMap<GUID, SResourceRecord*, skr::Hash<GUID>> resourceRecords;
    skr::ParallelFlatHashMap<void*, SResourceRecord*> resourceToRecord;
    skr::ParallelFlatHashMap<GUID, ResourceFactory*, skr::Hash<GUID>> resourceFactories;
};

ResourceSystemImpl::ResourceSystemImpl()
    : counter(true)
{
}

ResourceSystemImpl::~ResourceSystemImpl()
{
}

SResourceRecord* ResourceSystemImpl::FindResourceRecord(const GUID& guid)
{
    auto iter = resourceRecords.find(guid);
    return iter == resourceRecords.end() ? nullptr : iter->second;
}

SResourceRecord* ResourceSystemImpl::_GetRecord(void* resource)
{
    auto iter = resourceToRecord.find(resource);
    return iter == resourceToRecord.end() ? nullptr : iter->second;
}

void ResourceSystemImpl::_DestroyRecord(SResourceRecord* record)
{
    SMutexLock Lock(recordMutex.mMutex);
    auto request = static_cast<SResourceRequestImpl*>(record->activeRequest);
    if (request)
        request->resourceRecord = nullptr;
    resourceRecords.erase(record->header.guid);
    if (record->resource)
        resourceToRecord.erase(record->resource);
    SkrDelete(record);
}

ResourceFactory* ResourceSystemImpl::FindFactory(GUID type) const
{
    auto iter = resourceFactories.find(type);
    if (iter != resourceFactories.end()) return iter->second;
    return nullptr;
}

void ResourceSystemImpl::RegisterFactory(ResourceFactory* factory)
{
    auto type = factory->GetResourceType();
    auto iter = resourceFactories.find(type);
    SKR_ASSERT(iter == resourceFactories.end());
    resourceFactories.insert(std::make_pair(type, factory));
}

ResourceRegistry* ResourceSystemImpl::GetRegistry() const
{
    return resourceRegistry;
}

skr::io::IRAMService* ResourceSystemImpl::GetRAMService() const
{
    return ioService;
}

void ResourceSystemImpl::UnregisterFactory(GUID type)
{
    auto iter = resourceFactories.find(type);
    SKR_ASSERT(iter != resourceFactories.end());
    resourceFactories.erase(iter);
}

SResourceRecord* ResourceSystemImpl::_GetOrCreateRecord(const GUID& guid)
{
    SMutexLock Lock(recordMutex.mMutex);
    auto record = FindResourceRecord(guid);
    if (!record)
    {
        record = SkrNew<SResourceRecord>();
        record->header.guid = guid;
        resourceRecords.insert(std::make_pair(guid, record));
    }
    return record;
}

SResourceRecord* ResourceSystemImpl::LoadResource(const SResourceHandle& handle, bool requireInstalled)
{
    SKR_ASSERT(!quit);
    SKR_ASSERT(handle.is_guid());
    auto record = _GetOrCreateRecord(handle.get_guid());
    if (record->loadingStatus >= EResourceLoadingStatus::Loading) // already loaded
        return record;
        
    if (auto request = static_cast<SResourceRequestImpl*>(record->activeRequest))
    {
        request->requestInstall = requireInstalled;
    }
    else
    {
        request = SkrNew<SResourceRequestImpl>();
        request->requestInstall = requireInstalled;
        request->resourceRecord = record;
        request->system = this;
        request->factory = nullptr;
        request->vfs = nullptr;
        record->activeRequest = request;
        record->loadingStatus = EResourceLoadingStatus::Loading;
        counter.add(1);
        requests.enqueue(request);
    }
    return record;
}

SResourceHandle ResourceSystemImpl::EnqueueResource(skr::GUID guid, skr::GUID type, void* resource, bool requireInstalled, skr::Vector<SResourceHandle> dependencies, EResourceLoadingStatus status)
{
    SResourceHandle handle = guid;
    SResourceRecord* record = FindResourceRecord(guid);
    if (record == nullptr)
    {
        record = _GetOrCreateRecord(guid);
    }
    handle.acquire_record(record);

    if (auto request = static_cast<SResourceRequestImpl*>(record->activeRequest))
    {
        request->requestInstall = requireInstalled;
    }
    else
    {
        request = SkrNew<SResourceRequestImpl>();
        request->requestInstall = requireInstalled;
        request->resourceRecord = record;
        request->system = this;
        request->factory = FindFactory(type);
        counter.add(1);
        requests.enqueue(request);
    
        record->activeRequest = request;
        record->loadingStatus = status;
        record->resource = resource;
        record->header.guid = guid;
        record->header.type = type;
        record->header.dependencies = dependencies;
    }
    return handle;
}

void ResourceSystemImpl::UnloadResource(const SResourceHandle& handle)
{
    if (quit) return;

    SKR_ASSERT(!handle.is_null());
    if (auto record = handle.get_record())
    {
        SKR_ASSERT(record->loadingStatus != EResourceLoadingStatus::Unloaded);
        record->RemoveReference();
        memset((void*)&handle, 0, sizeof(SResourceHandle));
        if (!record->IsReferenced()) // unload
        {
            _UnloadResource(record);
        }
    }
}

void ResourceSystemImpl::_UnloadResource(SResourceRecord* record)
{
    SKR_ASSERT(!quit);
    if (record->loadingStatus == EResourceLoadingStatus::Error || record->loadingStatus == EResourceLoadingStatus::Unloading)
    {
        _DestroyRecord(record);
        return;
    }
    if (auto request = static_cast<SResourceRequestImpl*>(record->activeRequest))
    {

    }
    else // new unload
    {
        request = SkrNew<SResourceRequestImpl>();
        request->requestInstall = false;
        request->resourceRecord = record;
        request->system = this;
        request->vfs = nullptr;
        if (record->loadingStatus == EResourceLoadingStatus::Installed)
        {
            record->loadingStatus = EResourceLoadingStatus::Uninstalling;
        }
        else if (record->loadingStatus == EResourceLoadingStatus::Loaded)
        {
            record->loadingStatus = EResourceLoadingStatus::Unloading;
        }
        else
            SKR_UNREACHABLE_CODE();
        request->factory = this->FindFactory(record->header.type);
        record->activeRequest = request;
        counter.add(1);
        requests.enqueue(request);
    }
}

void ResourceSystemImpl::FlushResource(const SResourceHandle& handle){
    // flush load handle
    SKR_UNIMPLEMENTED_FUNCTION()
}

void ResourceSystemImpl::Initialize(ResourceRegistry* provider, skr::io::IRAMService* service)
{
    SKR_ASSERT(provider);
    resourceRegistry = provider;
    ioService = service;
}

bool ResourceSystemImpl::IsInitialized()
{
    return resourceRegistry != nullptr;
}

void ResourceSystemImpl::Shutdown()
{
    for (auto& pair : resourceRecords)
    {
        auto record = pair.second;
        if (record->loadingStatus == EResourceLoadingStatus::Unloaded || record->loadingStatus == EResourceLoadingStatus::Error)
            continue;
        _UnloadResource(record);
    }
    _ClearFinishedRequests();
    quit = true;
    Update(); // fill toUpdateRequests once
    while (!toUpdateRequests.empty())
    {
        Update();
    }
    for (auto pair : resourceRecords)
    {
        auto record = pair.second;
        SKR_ASSERT(record->loadingStatus == EResourceLoadingStatus::Error || record->loadingStatus == EResourceLoadingStatus::Unloaded);
        SkrDelete(record);
    }
    resourceRecords.clear();

    resourceRegistry = nullptr;
}

void ResourceSystemImpl::_ClearFinishedRequests()
{
    toUpdateRequests.erase(std::remove_if(toUpdateRequests.begin(), toUpdateRequests.end(), [&](ResourceRequest* req) {
        auto request = static_cast<SResourceRequestImpl*>(req);
        if (request->Okay())
        {
            if (request->resourceRecord)
            {
                request->resourceRecord->activeRequest = nullptr;
            }
            SkrDelete(request);
            counter.decrement();
            return true;
        }
        if (request->Failed())
        {
            failedRequests.push_back(req);
            counter.decrement();
            return true;
        }
        return false;
    }),
        toUpdateRequests.end());

    failedRequests.erase(std::remove_if(failedRequests.begin(), failedRequests.end(), [&](ResourceRequest* req) {
        auto request = static_cast<SResourceRequestImpl*>(req);
        if (!request->resourceRecord)
        {
            SkrDelete(request);
            return true;
        }
        return false;
    }),
        failedRequests.end());
}

void ResourceSystemImpl::Update()
{
    {
        ResourceRequest* request = nullptr;
        while (requests.try_dequeue(request))
        {
            toUpdateRequests.emplace_back(request);
        }
        _ClearFinishedRequests();
    }
    // TODO: time limit
    {
        for (auto req : toUpdateRequests)
        {
            auto request = static_cast<SResourceRequestImpl*>(req);
            auto record = request->resourceRecord;
            uint32_t spinCounter = 0;
            EResourceLoadingStatus LastPhase;
            while (!request->Okay()&& spinCounter < 16)
            {
                LastPhase = record->loadingStatus;
                request->Update();
                if (LastPhase == record->loadingStatus)
                    spinCounter++;
                else
                    spinCounter = 0;
            };
        }
    }
}

bool ResourceSystemImpl::WaitRequest()
{
    if (quit)
        return false;
    counter.wait(true);
    return !quit;
}

void ResourceSystemImpl::Quit()
{
    quit = true;
    counter.add(1);
}

ResourceSystem* GetResourceSystem()
{
    static ResourceSystemImpl system;
    return &system;
}
} // namespace skr