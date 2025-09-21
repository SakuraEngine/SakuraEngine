#include "resource_request_impl.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrContainersDef/stl_vector.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrRuntime/resource/resource_factory.h"
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

    void LoadResource(SResourceHandle& handle, bool requireInstalled, uint64_t requester, ESkrRequesterType) final override;
    void UnloadResource(SResourceHandle& handle) final override;
    void _UnloadResource(SResourceRecord* record);
    void FlushResource(SResourceHandle& handle) final override;
    ESkrLoadingStatus GetResourceStatus(const GUID& handle) final override;

    ResourceFactory* FindFactory(GUID type) const final override;
    void RegisterFactory(ResourceFactory* factory) final override;
    void UnregisterFactory(GUID type) final override;

    ResourceRegistry* GetRegistry() const final override;
    skr::io::IRAMService* GetRAMService() const final override;

protected:
    SResourceRecord* _GetOrCreateRecord(const GUID& guid) final override;
    SResourceRecord* _GetRecord(const GUID& guid) final override;
    SResourceRecord* _GetRecord(void* resource) final override;
    void _DestroyRecord(SResourceRecord* record) final override;
    void _UpdateAsyncSerde();
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

    sugoi::EntityRegistry resourceIds;
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

SResourceRecord* ResourceSystemImpl::_GetOrCreateRecord(const GUID& guid)
{
    SMutexLock Lock(recordMutex.mMutex);
    auto record = _GetRecord(guid);
    if (!record)
    {
        record = SkrNew<SResourceRecord>();
        resourceIds.new_entities(&record->id, 1);
        record->header.guid = guid;
        // record->header.type = type;
        resourceRecords.insert(std::make_pair(guid, record));
    }
    return record;
}

SResourceRecord* ResourceSystemImpl::_GetRecord(const GUID& guid)
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
    resourceIds.free_entities(&record->id, 1);
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

void ResourceSystemImpl::LoadResource(SResourceHandle& handle, bool requireInstalled, uint64_t requester, ESkrRequesterType requesterType)
{
    SKR_ASSERT(!quit);
    SKR_ASSERT(!handle.is_resolved());
    auto record = _GetOrCreateRecord(handle.get_guid());
    auto requesterId = record->AddReference(requester, requesterType);
    handle.set_resolved(record, requesterId, requesterType);
    if ((!requireInstalled && record->loadingStatus >= SKR_LOADING_STATUS_LOADED && record->loadingStatus < SKR_LOADING_STATUS_UNLOADING) ||
        (requireInstalled && record->loadingStatus == SKR_LOADING_STATUS_INSTALLED) ||
        record->loadingStatus == SKR_LOADING_STATUS_ERROR) // already loaded
        return;
    auto request = static_cast<SResourceRequestImpl*>(record->activeRequest);
    if (request)
    {
        request->requireLoading = true;
        request->requestInstall = requireInstalled;
    }
    else
    {
        auto request = SkrNew<SResourceRequestImpl>();
        request->requestInstall = requireInstalled;
        request->resourceRecord = record;
        request->isLoading = request->requireLoading = true;
        request->system = this;
        request->currentPhase = SKR_LOADING_PHASE_REQUEST_RESOURCE;
        request->factory = nullptr;
        request->vfs = nullptr;
        record->activeRequest = request;
        record->loadingStatus = SKR_LOADING_STATUS_LOADING;
        counter.add(1);
        requests.enqueue(request);
    }
}

void ResourceSystemImpl::UnloadResource(SResourceHandle& handle)
{
    if (quit)
        return;
    SKR_ASSERT(handle.is_resolved() && !handle.is_null());
    auto record = handle.get_record();
    SKR_ASSERT(record->loadingStatus != SKR_LOADING_STATUS_UNLOADED);
    record->RemoveReference(handle.get_requester_id(), handle.get_requester_type());
    // reset to zero otherwise set_guid will trigger unload again
    memset((void*)&handle, 0, sizeof(SResourceHandle));
    handle.set_guid(record->header.guid);
    if (!record->IsReferenced()) // unload
    {
        _UnloadResource(record);
    }
}

void ResourceSystemImpl::_UnloadResource(SResourceRecord* record)
{
    SKR_ASSERT(!quit);
    if (record->loadingStatus == SKR_LOADING_STATUS_ERROR || record->loadingStatus == SKR_LOADING_STATUS_UNLOADED)
    {
        _DestroyRecord(record);
        return;
    }
    auto request = static_cast<SResourceRequestImpl*>(record->activeRequest);
    if (request)
    {
        request->requireLoading = false;
    }
    else // new unload
    {
        auto request = SkrNew<SResourceRequestImpl>();
        request->requestInstall = false;
        request->resourceRecord = record;
        request->isLoading = request->requireLoading = false;
        request->system = this;
        request->vfs = nullptr;
        if (record->loadingStatus == SKR_LOADING_STATUS_INSTALLED)
        {
            request->currentPhase = SKR_LOADING_PHASE_UNINSTALL_RESOURCE;
            record->loadingStatus = SKR_LOADING_STATUS_UNINSTALLING;
        }
        else if (record->loadingStatus == SKR_LOADING_STATUS_LOADED)
        {
            request->currentPhase = SKR_LOADING_PHASE_UNLOAD_RESOURCE;
            record->loadingStatus = SKR_LOADING_STATUS_UNLOADING;
        }
        else
            SKR_UNREACHABLE_CODE();
        request->factory = this->FindFactory(record->header.type);
        record->activeRequest = request;
        counter.add(1);
        requests.enqueue(request);
    }
}

void ResourceSystemImpl::FlushResource(SResourceHandle& handle){
    // flush load handle
    SKR_UNIMPLEMENTED_FUNCTION()
}

ESkrLoadingStatus ResourceSystemImpl::GetResourceStatus(const GUID& handle)
{
    SMutexLock Lock(recordMutex.mMutex);
    auto record = _GetRecord(handle);
    if (!record) return SKR_LOADING_STATUS_UNLOADED;
    return record->loadingStatus;
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
        if (record->loadingStatus == SKR_LOADING_STATUS_ERROR || record->loadingStatus == SKR_LOADING_STATUS_UNLOADED)
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
        SKR_ASSERT(record->loadingStatus == SKR_LOADING_STATUS_ERROR || record->loadingStatus == SKR_LOADING_STATUS_UNLOADED);
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
                if (!request->isLoading)
                {
                    auto guid = request->resourceRecord->header.guid;
                    (void)guid;
                    _DestroyRecord(request->resourceRecord);
                }
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
            uint32_t spinCounter = 0;
            ESkrLoadingPhase LastPhase;
            while (!request->Okay() && !request->AsyncSerde() && spinCounter < 16)
            {
                LastPhase = request->currentPhase;
                request->Update();
                if (LastPhase == request->currentPhase)
                    spinCounter++;
                else
                    spinCounter = 0;
            };
        }
    }
    _UpdateAsyncSerde();
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

void ResourceSystemImpl::_UpdateAsyncSerde()
{
    serdeBatch.clear();
    serdeBatch.reserve(100);
    float timeBudget = 100.f;
    for (auto req : toUpdateRequests)
    {
        auto request = static_cast<SResourceRequestImpl*>(req);
        if (request->currentPhase == SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE && !request->serdeScheduled)
        {
            request->serdeScheduled = true;
            auto factor = request->factory->AsyncSerdeLoadFactor();
            timeBudget -= factor;
            serdeBatch.push_back(request);
            if (timeBudget < 0.f)
            {
                timeBudget = 0.f;
                skr::task::schedule([batch = std::move(serdeBatch)]() {
                    for (auto request : batch)
                    {
                        request->LoadTask();
                    }
                },
                    nullptr);
                timeBudget = 100.f;
            }
        }
    }
    if (!serdeBatch.empty())
    {
        // run rest requests on main thread
        for (auto request : serdeBatch)
        {
            request->LoadTask();
        }
    }
}

ResourceSystem* GetResourceSystem()
{
    static ResourceSystemImpl system;
    return &system;
}
} // namespace skr