#include "resource_request_impl.hpp"
#include "async/fib_task.hpp"
#include "platform/guid.hpp"
#include "ecs/entities.hpp"
#include "platform/debug.h"
#include "containers/hashmap.hpp"
#include "misc/io.h"
#include "platform/vfs.h"
#include "resource/resource_factory.h"
#include "containers/concurrent_queue.h"

namespace skr::resource
{
struct RUNTIME_API SResourceSystemImpl : public SResourceSystem
{
    friend struct ::skr_resource_handle_t;
public:
    SResourceSystemImpl();
    ~SResourceSystemImpl();
    void Initialize(SResourceRegistry* provider, skr_io_ram_service_t* ioService) final override;
    bool IsInitialized() final override;
    void Shutdown() final override;
    void Update() final override;
    bool WaitRequest() final override;
    void Quit() final override;

    void LoadResource(skr_resource_handle_t& handle, bool requireInstalled, uint64_t requester, ESkrRequesterType) final override;
    void UnloadResource(skr_resource_handle_t& handle) final override;
    void _UnloadResource(skr_resource_record_t* record);
    void FlushResource(skr_resource_handle_t& handle) final override;
    ESkrLoadingStatus GetResourceStatus(const skr_guid_t& handle) final override;

    SResourceFactory* FindFactory(skr_type_id_t type) const final override;
    void RegisterFactory(SResourceFactory* factory) final override;
    void UnregisterFactory(skr_type_id_t type) final override;

    SResourceRegistry* GetRegistry() const final override;
    skr_io_ram_service_t* GetRAMService() const final override;

protected:
    skr_resource_record_t* _GetOrCreateRecord(const skr_guid_t& guid) final override;
    skr_resource_record_t* _GetRecord(const skr_guid_t& guid) final override;
    skr_resource_record_t* _GetRecord(void* resource) final override;
    void _DestroyRecord(skr_resource_record_t* record) final override;
    void _UpdateAsyncSerde();
    void _ClearFinishedRequests();

    SResourceRegistry* resourceRegistry = nullptr;
    skr_io_ram_service_t* ioService = nullptr; 

    struct ResourceRequestConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
    {
        static constexpr const char* kResourceRequestQueueName = "";
        static const bool RECYCLE_ALLOCATED_BLOCKS = true;
        static inline void* malloc(size_t size) { return sakura_mallocN(size, kResourceRequestQueueName); }
        static inline void free(void* ptr) { return sakura_freeN(ptr, kResourceRequestQueueName); }
    };

    skr::ConcurrentQueue<SResourceRequest*, ResourceRequestConcurrentQueueTraits> requests;
    SMutexObject recordMutex; // this mutex is used to protect the resourceRecords and resourceToRecord maps

    // these requests are only handled inside this system and is thread-unsafe

    eastl::vector<SResourceRequest*> failedRequests;
    eastl::vector<SResourceRequest*> toUpdateRequests;
    eastl::vector<SResourceRequest*> serdeBatch;

    dual::entity_registry_t resourceIds;
    task::counter_t counter;
    bool quit = false;
    skr::parallel_flat_hash_map<skr_guid_t, skr_resource_record_t*, skr::guid::hash> resourceRecords;
    skr::parallel_flat_hash_map<void*, skr_resource_record_t*> resourceToRecord;
    skr::parallel_flat_hash_map<skr_type_id_t, SResourceFactory*, skr::guid::hash> resourceFactories;
};

SResourceSystemImpl::SResourceSystemImpl()
    : counter(true)
{

}

SResourceSystemImpl::~SResourceSystemImpl()
{

}

skr_resource_record_t* SResourceSystemImpl::_GetOrCreateRecord(const skr_guid_t& guid)
{
    SMutexLock Lock(recordMutex.mMutex);
    auto record = _GetRecord(guid);
    if (!record)
    {
        record = SkrNew<skr_resource_record_t>();
        resourceIds.new_entities(&record->id, 1);
        record->header.guid = guid;
        // record->header.type = type;
        resourceRecords.insert(std::make_pair(guid, record));
    }
    return record;
}

skr_resource_record_t* SResourceSystemImpl::_GetRecord(const skr_guid_t& guid)
{
    auto iter = resourceRecords.find(guid);
    return iter == resourceRecords.end() ? nullptr : iter->second;
}

skr_resource_record_t* SResourceSystemImpl::_GetRecord(void* resource)
{
    auto iter = resourceToRecord.find(resource);
    return iter == resourceToRecord.end() ? nullptr : iter->second;
}

void SResourceSystemImpl::_DestroyRecord(skr_resource_record_t* record)
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

SResourceFactory* SResourceSystemImpl::FindFactory(skr_type_id_t type) const
{
    auto iter = resourceFactories.find(type);
    if (iter != resourceFactories.end()) return iter->second;
    return nullptr;
}

void SResourceSystemImpl::RegisterFactory(SResourceFactory* factory)
{
    auto type = factory->GetResourceType();
    auto iter = resourceFactories.find(type);
    SKR_ASSERT(iter == resourceFactories.end());
    resourceFactories.insert(std::make_pair(type, factory));
}

SResourceRegistry* SResourceSystemImpl::GetRegistry() const
{
    return resourceRegistry;
}

skr_io_ram_service_t* SResourceSystemImpl::GetRAMService() const
{
    return ioService;
}

void SResourceSystemImpl::UnregisterFactory(skr_type_id_t type)
{
    auto iter = resourceFactories.find(type);
    SKR_ASSERT(iter != resourceFactories.end());
    resourceFactories.erase(iter);
}

void SResourceSystemImpl::LoadResource(skr_resource_handle_t& handle, bool requireInstalled, uint64_t requester, ESkrRequesterType requesterType)
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

void SResourceSystemImpl::UnloadResource(skr_resource_handle_t& handle)
{
    if(quit)
        return;
    SKR_ASSERT(handle.is_resolved() && !handle.is_null());
    auto record = handle.get_record();
    SKR_ASSERT(record->loadingStatus != SKR_LOADING_STATUS_UNLOADED);
    record->RemoveReference(handle.get_requester_id(), handle.get_requester_type());
    auto guid = handle.guid = record->header.guid; (void)guid;// force flush handle to guid
    if (!record->IsReferenced()) // unload
    {
        _UnloadResource(record);
    }
}


void SResourceSystemImpl::_UnloadResource(skr_resource_record_t* record)
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

void SResourceSystemImpl::FlushResource(skr_resource_handle_t& handle)
{
    // flush load handle
    SKR_UNIMPLEMENTED_FUNCTION()
}

ESkrLoadingStatus SResourceSystemImpl::GetResourceStatus(const skr_guid_t& handle)
{
    SMutexLock Lock(recordMutex.mMutex);
    auto record = _GetRecord(handle);
    if (!record) return SKR_LOADING_STATUS_UNLOADED;
    return record->loadingStatus;
}

void SResourceSystemImpl::Initialize(SResourceRegistry* provider, skr_io_ram_service_t* service)
{
    SKR_ASSERT(provider);
    resourceRegistry = provider;
    ioService = service;
}

bool SResourceSystemImpl::IsInitialized()
{
    return resourceRegistry != nullptr;
}

void SResourceSystemImpl::Shutdown()
{
    for(auto& pair : resourceRecords)
    {
        auto record = pair.second;
        if (record->loadingStatus == SKR_LOADING_STATUS_ERROR || record->loadingStatus == SKR_LOADING_STATUS_UNLOADED)
            continue;
        _UnloadResource(record);
    }
    _ClearFinishedRequests();
    quit = true;
    Update(); // fill toUpdateRequests once
    while(!toUpdateRequests.empty())
    {
        Update();
    }
    for(auto pair : resourceRecords)
    {
        auto record = pair.second;
        SKR_ASSERT(record->loadingStatus == SKR_LOADING_STATUS_ERROR || record->loadingStatus == SKR_LOADING_STATUS_UNLOADED);
        SkrDelete(record);
    }
    resourceRecords.clear();

    resourceRegistry = nullptr;
}

void SResourceSystemImpl::_ClearFinishedRequests()
{
    toUpdateRequests.erase(std::remove_if(toUpdateRequests.begin(), toUpdateRequests.end(), 
        [&](SResourceRequest* req) {
        auto request = static_cast<SResourceRequestImpl*>(req);
        if (request->Okay())
        {
            if (request->resourceRecord)
            {
                request->resourceRecord->activeRequest = nullptr;
                if (!request->isLoading)
                {
                    auto guid = request->resourceRecord->header.guid; (void)guid;
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
    
    failedRequests.erase(std::remove_if(failedRequests.begin(), failedRequests.end(), 
    [&](SResourceRequest* req) {
        auto request = static_cast<SResourceRequestImpl*>(req);
        if(!request->resourceRecord)
        {
            SkrDelete(request);
            return true;
        }
        return false;
    }), failedRequests.end());
}

void SResourceSystemImpl::Update()
{
    {
        SResourceRequest* request = nullptr;
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
            while(!request->Okay() && !request->AsyncSerde() && spinCounter < 16)
            {
                LastPhase = request->currentPhase;
                request->Update();
                if(LastPhase == request->currentPhase)
                    spinCounter++;
                else
                    spinCounter = 0;
            };
        }
    }
    _UpdateAsyncSerde();
}


bool SResourceSystemImpl::WaitRequest()
{
    if(quit)
        return false;
    counter.wait(true);
    return !quit;
}

void SResourceSystemImpl::Quit()
{
    quit = true;
    counter.add(1);
}

void SResourceSystemImpl::_UpdateAsyncSerde()
{
    serdeBatch.clear();
    serdeBatch.reserve(100);
    float timeBudget = 100.f;
    for (auto req : toUpdateRequests)
    {
        auto request = static_cast<SResourceRequestImpl*>(req);
        if(request->currentPhase == SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE && !request->serdeScheduled)
        {
            request->serdeScheduled = true;
            auto factor = request->factory->AsyncSerdeLoadFactor();
            timeBudget -= factor;
            serdeBatch.push_back(request);
            if(timeBudget < 0.f)
            {
                timeBudget = 0.f;
                skr::task::schedule([batch = std::move(serdeBatch)](){
                    for(auto request : batch)
                    {
                        request->LoadTask();
                    }
                }, nullptr);
                timeBudget = 100.f;
            }
        }
    }
    if(!serdeBatch.empty())
    {
        // run rest requests on main thread
        for(auto request : serdeBatch)
        {
            request->LoadTask();
        }
    }
}

SResourceSystem* GetResourceSystem()
{
    static SResourceSystemImpl system;
    return &system;
}
} // namespace skr::resource