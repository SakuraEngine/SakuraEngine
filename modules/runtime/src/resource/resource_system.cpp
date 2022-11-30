#include "platform/debug.h"
#include "platform/thread.h"
#include "containers/hashmap.hpp"
#include "utils/defer.hpp"
#include "utils/io.hpp"
#include "utils/log.hpp"
#include "resource/resource_system.h"
#include "resource/resource_handle.h"
#include "resource/resource_header.hpp"
#include "platform/vfs.h"
#include "resource/resource_factory.h"

namespace skr::resource
{
struct RUNTIME_API SResourceSystemImpl : public SResourceSystem
{
    friend struct ::skr_resource_handle_t;
public:
    SResourceSystemImpl();
    ~SResourceSystemImpl();
    void Initialize(SResourceRegistry* provider, skr::io::RAMService* ioService) final override;
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
    skr::io::RAMService* GetRAMService() const final override;

protected:
    skr_resource_record_t* _GetOrCreateRecord(const skr_guid_t& guid) final override;
    skr_resource_record_t* _GetRecord(const skr_guid_t& guid) final override;
    skr_resource_record_t* _GetRecord(void* resource) final override;
    void _DestroyRecord(skr_resource_record_t* record) final override;
    void _UpdateAsyncSerde();

    SResourceRegistry* resourceRegistry = nullptr;
    skr::io::RAMService* ioService = nullptr; 
    eastl::vector<SResourceRequest*> requests;
    eastl::vector<SResourceRequest*> failedRequests;
    dual::entity_registry_t resourceIds;
    SMutex recordMutex;
    task::counter_t counter;
    bool quit = false;
    eastl::vector<SResourceRequest*> serdeBatch;
    skr::flat_hash_map<skr_guid_t, skr_resource_record_t*, skr::guid::hash> resourceRecords;
    skr::flat_hash_map<void*, skr_resource_record_t*> resourceToRecord;
    skr::flat_hash_map<skr_type_id_t, SResourceFactory*, skr::guid::hash> resourceFactories;
};

SResourceSystemImpl::SResourceSystemImpl()
    : counter(true)
{
    skr_init_mutex(&recordMutex);
}

SResourceSystemImpl::~SResourceSystemImpl()
{
    skr_destroy_mutex(&recordMutex);
}

skr_resource_record_t* SResourceSystemImpl::_GetOrCreateRecord(const skr_guid_t& guid)
{
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
    //SMutexLock lock(recordMutex);
    auto request = record->activeRequest;
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

skr::io::RAMService* SResourceSystemImpl::GetRAMService() const
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
    SMutexLock lock(recordMutex);
    SKR_ASSERT(!handle.is_resolved());
    auto record = _GetOrCreateRecord(handle.get_guid());
    auto requesterId = record->AddReference(requester, requesterType);
    handle.set_resolved(record, requesterId, requesterType);
    if ((!requireInstalled && record->loadingStatus >= SKR_LOADING_STATUS_LOADED && record->loadingStatus < SKR_LOADING_STATUS_UNLOADING) ||
        (requireInstalled && record->loadingStatus == SKR_LOADING_STATUS_INSTALLED) ||
        record->loadingStatus == SKR_LOADING_STATUS_ERROR) // already loaded
        return;
    auto request = record->activeRequest;
    if (request)
    {
        request->requireLoading = true;
        request->requestInstall = requireInstalled;
    }
    else
    {
        auto request = SkrNew<SResourceRequest>();
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
        requests.push_back(request);
    }
}

void SResourceSystemImpl::UnloadResource(skr_resource_handle_t& handle)
{
    SMutexLock lock(recordMutex);
    SKR_ASSERT(handle.is_resolved() && !handle.is_null());
    auto record = handle.get_record();
    SKR_ASSERT(record->loadingStatus != SKR_LOADING_STATUS_UNLOADED);
    record->RemoveReference(handle.get_requester_id(), handle.get_requester_type());
    auto guid = handle.guid = record->header.guid; (void)guid;// force flush handle to guid
    _UnloadResource(record);
}


void SResourceSystemImpl::_UnloadResource(skr_resource_record_t* record)
{
    if (!record->IsReferenced()) // unload
    {
        if (record->loadingStatus == SKR_LOADING_STATUS_ERROR || record->loadingStatus == SKR_LOADING_STATUS_UNLOADED)
        {
            _DestroyRecord(record);
            return;
        }
        auto request = record->activeRequest;
        if (request)
        {
            request->requireLoading = false;
        }
        else // new unload
        {
            auto request = SkrNew<SResourceRequest>();
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
            requests.push_back(request);
        }
    }
}

void SResourceSystemImpl::FlushResource(skr_resource_handle_t& handle)
{
    // flush load handle
    SKR_UNIMPLEMENTED_FUNCTION()
}

ESkrLoadingStatus SResourceSystemImpl::GetResourceStatus(const skr_guid_t& handle)
{
    SMutexLock lock(recordMutex);
    auto record = _GetRecord(handle);
    if (!record) return SKR_LOADING_STATUS_UNLOADED;
    return record->loadingStatus;
}

void SResourceSystemImpl::Initialize(SResourceRegistry* provider, skr::io::RAMService* service)
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
        if(!pair.second->activeRequest)
        {
            _UnloadResource(record);
        }
    }
    while(!requests.empty())
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

void SResourceSystemImpl::Update()
{
    SKR_ASSERT(!quit);
    eastl::vector<SResourceRequest*> to_update_requests;
    {
        SMutexLock lock(recordMutex);
        requests.erase(std::remove_if(requests.begin(), requests.end(), [&](SResourceRequest* request) {
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
                failedRequests.push_back(request);
                counter.decrement();
                return true;
            }
            return false;
        }),
        requests.end());
        failedRequests.erase(std::remove_if(failedRequests.begin(), failedRequests.end(), [&](SResourceRequest* request) {
            if(!request->resourceRecord)
            {
                SkrDelete(request);
                return true;
            }
            return false;
        }), failedRequests.end());
        to_update_requests = requests;
    }
    // TODO: time limit
    {
        for (auto request : to_update_requests)
        {
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
    for (auto request : requests)
    {
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