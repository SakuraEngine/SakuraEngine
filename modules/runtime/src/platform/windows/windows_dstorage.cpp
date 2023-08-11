#include "cgpu/extensions/cgpu_d3d12_exts.h"
#include "SkrRT/platform/win/misc.h"
#include "SkrRT/platform/filesystem.hpp"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/misc/defer.hpp"
#include "SkrRT/containers/vector.hpp"
#include "SkrRT/containers/concurrent_queue.h"

#include "platform/windows/windows_dstorage.hpp"
#include "EASTL/algorithm.h"

struct StatusEventArray;
SkrWindowsDStorageInstance* SkrWindowsDStorageInstance::_this = nullptr;

struct SkrDStorageEvent
{
    SkrDStorageEvent(StatusEventArray* array, uint32_t slot) SKR_NOEXCEPT
        : pArray(array), mSlot(slot) 
    {

    }

    bool test() SKR_NOEXCEPT;
    bool get_enqueued() SKR_NOEXCEPT { return mEnqueued; }
    void enqueue_status(IDStorageQueue* Q) SKR_NOEXCEPT;

protected:
    friend struct StatusEventArray;
    friend struct DStorageEventPool;
    StatusEventArray* pArray = nullptr;
    uint32_t mSlot = 0;
    bool mEnqueued = false;
};

struct StatusEventArray
{
    StatusEventArray(IDStorageStatusArray* pArray) SKR_NOEXCEPT
        : pArray(pArray)
    {
        for (uint32_t i = 0; i < kMaxEvents; i++)
            freeSlots.enqueue(i);
    }

    ~StatusEventArray() SKR_NOEXCEPT
    {
        if (pArray)
            pArray->Release();
    }

    SkrDStorageEvent* Allocate() SKR_NOEXCEPT
    {
        uint32_t slot = 0;
        if (freeSlots.try_dequeue(slot))
        {
            return SkrNew<SkrDStorageEvent>(this, slot);
        }
        return nullptr;
    }

    void Deallocate(SkrDStorageEvent* e) SKR_NOEXCEPT
    {
        freeSlots.enqueue(e->mSlot);
        SkrDelete(e);
    }
    static const uint32_t kMaxEvents = 1024;
protected:
    friend struct SkrDStorageEvent;
    friend struct DStorageEventPool;
    IDStorageStatusArray* pArray = nullptr;
    skr::ConcurrentQueue<uint32_t> freeSlots;
};

bool SkrDStorageEvent::test() SKR_NOEXCEPT
{
    const auto c = pArray->pArray->IsComplete(mSlot);
    return c;
}

void SkrDStorageEvent::enqueue_status(IDStorageQueue* Q) SKR_NOEXCEPT
{
    Q->EnqueueStatus(pArray->pArray, mSlot);
    mEnqueued = true;
}

struct DStorageEventPool
{
    DStorageEventPool(IDStorageFactory* pFactory) SKR_NOEXCEPT
        : pFactory(pFactory)
    {
        pFactory->AddRef();
        skr_init_rw_mutex(&arrMutex);
        addArray();
    }
    
    ~DStorageEventPool() SKR_NOEXCEPT
    {
        removeAllArrays();
        pFactory->Release();
        skr_destroy_rw_mutex(&arrMutex);
    }

    SkrDStorageEvent* Allocate() SKR_NOEXCEPT
    {
        if (auto e = tryAllocate())
            return e;
        addArray();
        return tryAllocate();
    }

    void Deallocate(SkrDStorageEvent* e) SKR_NOEXCEPT
    {
        if (!e) return;
        SKR_ASSERT(e->pArray && "Invalid event");
        if (e->pArray)
        {
            e->pArray->Deallocate(e);
        }
    }

private:
    SkrDStorageEvent* tryAllocate() SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_r(&arrMutex);
        SKR_DEFER({ skr_rw_mutex_release_r(&arrMutex); });
        for (auto arr : statusArrays)
        {
            if (auto e = arr->Allocate())
                return e;
        }
        return nullptr;
    }

    void addArray() SKR_NOEXCEPT
    {
        IDStorageStatusArray* pArray = nullptr;
        pFactory->CreateStatusArray(StatusEventArray::kMaxEvents, "DirectStorageEvents", IID_PPV_ARGS(&pArray));
        {
            skr_rw_mutex_acquire_w(&arrMutex);
            SKR_DEFER({ skr_rw_mutex_release_w(&arrMutex); });
            statusArrays.emplace_back(SkrNew<StatusEventArray>(pArray));
        }
    }

    void removeArray(StatusEventArray* pArray) SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_w(&arrMutex);
        SKR_DEFER({ skr_rw_mutex_release_w(&arrMutex); });
        auto it = eastl::find(statusArrays.begin(), statusArrays.end(), pArray);
        if (it != statusArrays.end())
        {
            SkrDelete(*it);
            statusArrays.erase(it);
        }
    }

    void removeAllArrays() SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_w(&arrMutex);
        SKR_DEFER({ skr_rw_mutex_release_w(&arrMutex); });
        for (auto& arr : statusArrays)
        {
            SkrDelete(arr);
        }
        statusArrays.clear();
    }

    SRWMutex arrMutex;
    skr::vector<StatusEventArray*> statusArrays;
    IDStorageFactory* pFactory = nullptr;
};

SkrWindowsDStorageInstance* SkrWindowsDStorageInstance::Get()
{
    return _this;
}

SkrWindowsDStorageInstance* SkrWindowsDStorageInstance::Initialize(const SkrDStorageConfig& cfg)
{
    const bool wine = skr_win_is_wine();
    if (!_this)
    {
        _this = SkrNew<SkrWindowsDStorageInstance>();
        if (wine)
        {
            _this->initialize_failed = true;
        }
        else
        {
            _this->dstorage_core.load(u8"dstoragecore.dll");
            _this->dstorage_library.load(u8"dstorage.dll");
            if (!_this->dstorage_core.isLoaded() || !_this->dstorage_library.isLoaded())
            {
                if (!_this->dstorage_core.isLoaded()) SKR_LOG_TRACE(u8"dstoragecore.dll not found, direct storage is disabled");
                if (!_this->dstorage_library.isLoaded()) SKR_LOG_TRACE(u8"dstorage.dll not found, direct storage is disabled");
                _this->initialize_failed = true;
            }
            else
            {
                SKR_LOG_TRACE(u8"dstorage.dll loaded");

                auto pfn_set_config = SKR_SHARED_LIB_LOAD_API(_this->dstorage_library, DStorageSetConfiguration);
                if (!pfn_set_config) return nullptr;

                auto pfn_set_config1 = SKR_SHARED_LIB_LOAD_API(_this->dstorage_library, DStorageSetConfiguration1);
                if (!pfn_set_config1) return nullptr;
                auto config1 = make_zeroed<DSTORAGE_CONFIGURATION1>();
                config1.DisableBypassIO = cfg.no_bypass;
                config1.ForceFileBuffering = cfg.enable_cache;
                if (const bool hdd = !skr_win_is_executable_on_ssd())
                {
                    // force file buffering on HDD
                    config1.DisableBypassIO = true;
                    config1.ForceFileBuffering = true;
                }
                if (!SUCCEEDED(pfn_set_config1(&config1)))
                {
                    SKR_LOG_ERROR(u8"Failed to set DStorage config!");
                    return nullptr;
                }

                auto pfn_get_factory = SKR_SHARED_LIB_LOAD_API(_this->dstorage_library, DStorageGetFactory);
                if (!pfn_get_factory) return nullptr;
                if (!SUCCEEDED(pfn_get_factory(IID_PPV_ARGS(&_this->pFactory))))
                {
                    SKR_LOG_ERROR(u8"Failed to get DStorage factory!");
                    return nullptr;
                }

                _this->pEventPool = SkrNew<DStorageEventPool>(_this->pFactory);
            }
        }
    }
    return _this->initialize_failed ? nullptr : _this;
}

SkrWindowsDStorageInstance::~SkrWindowsDStorageInstance()
{
    if (_this->pEventPool) 
        SkrDelete(_this->pEventPool);
    if (pFactory) 
        pFactory->Release();
    if (pDxDevice) 
        pDxDevice->Release();
    if (dstorage_library.isLoaded()) 
        dstorage_library.unload();
    if (dstorage_core.isLoaded()) 
        dstorage_core.unload();
    
    SKR_LOG_TRACE(u8"Direct Storage unloaded");
}

SkrDStorageInstanceId skr_create_dstorage_instance(SkrDStorageConfig* config)
{
    return SkrWindowsDStorageInstance::Initialize(*config);
}

SkrDStorageInstanceId skr_get_dstorage_instnace()
{
    auto created = SkrWindowsDStorageInstance::Get();
    SKR_ASSERT(created && 
        "Direct Storage instance not created, "
        "you must call 'skr_create_dstorage_instance' at first!");
    return created;
}

ESkrDStorageAvailability skr_query_dstorage_availability()
{
    if (auto inst = SkrWindowsDStorageInstance::Get())
    {
        return inst->initialize_failed ? SKR_DSTORAGE_AVAILABILITY_NONE : SKR_DSTORAGE_AVAILABILITY_HARDWARE;
    }
    return SKR_DSTORAGE_AVAILABILITY_NONE;
}

void skr_free_dstorage_instance(SkrDStorageInstanceId inst)
{
    SkrDelete((SkrWindowsDStorageInstance*)inst);
}

SkrDStorageQueueId skr_create_dstorage_queue(const SkrDStorageQueueDescriptor* desc)
{
    auto _this = (SkrWindowsDStorageInstance*)skr_get_dstorage_instnace();
    if (!_this) return nullptr;

    DStorageQueueWindows* Q = SkrNew<DStorageQueueWindows>();
    DSTORAGE_QUEUE_DESC queueDesc{};
    queueDesc.Capacity = desc->capacity;
    queueDesc.Priority = (DSTORAGE_PRIORITY)desc->priority;
    Q->source_type = queueDesc.SourceType = (DSTORAGE_REQUEST_SOURCE_TYPE)desc->source;
    queueDesc.Name = (const char*)desc->name;
    queueDesc.Device = nullptr;
    if (desc->gpu_device)
    {
        SKR_ASSERT(desc->gpu_device);
        queueDesc.Device = cgpu_d3d12_get_device(desc->gpu_device);
    }
    IDStorageFactory* pFactory = _this->pFactory;
    if (!pFactory) return nullptr;
    if (!SUCCEEDED(pFactory->CreateQueue(&queueDesc, IID_PPV_ARGS(&Q->pQueue))))
    {
        SKR_LOG_ERROR(u8"Failed to create DStorage queue!");
        SkrDelete(Q);
        return nullptr;
    }
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    skr_init_mutex_recursive(&Q->profile_mutex);
#endif
    Q->max_size = _this->sDirectStorageStagingBufferSize;
    Q->pFactory = pFactory;
    Q->pFactory->AddRef();
    Q->pDxDevice = _this->pDxDevice = queueDesc.Device ? queueDesc.Device : nullptr;
    if (Q->pDxDevice)
        Q->pDxDevice->AddRef();
    if (_this->pDxDevice)
        _this->pDxDevice->AddRef();
    Q->device = desc->gpu_device;
    Q->pInstance = _this;
    return Q;
}

void skr_free_dstorage_queue(SkrDStorageQueueId queue)
{
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    if (Q->pQueue)
    {
        // TODO: WaitErrorEvent & Handle errors 
        DSTORAGE_ERROR_RECORD record = {};
        Q->pQueue->RetrieveErrorRecord(&record);

#ifdef TRACY_PROFILE_DIRECT_STORAGE
        skr_destroy_mutex(&Q->profile_mutex);
#endif
        Q->pQueue->Release();
        if (Q->pDxDevice)
            Q->pDxDevice->Release();
        Q->pFactory->Release();
    }
    SkrDelete(Q);
}

SkrDStorageFileHandle skr_dstorage_open_file(SkrDStorageInstanceId inst, const char8_t* abs_path)
{
    IDStorageFile* pFile = nullptr;
    auto I = (SkrWindowsDStorageInstance*)inst;
    auto absPath = skr::filesystem::path(abs_path);
    I->pFactory->OpenFile(absPath.c_str(), IID_PPV_ARGS(&pFile));
    return (SkrDStorageFileHandle)pFile;
}

void skr_dstorage_query_file_info(SkrDStorageInstanceId inst, SkrDStorageFileHandle file, SkrDStorageFileInfo* info)
{
    BY_HANDLE_FILE_INFORMATION fileInfo;
    IDStorageFile* pFile = (IDStorageFile*)file;
    if (!SUCCEEDED(pFile->GetFileInformation(&fileInfo)))
    {
        SKR_LOG_ERROR(u8"Failed to get DStorage file info!");
        return;
    }
    info->file_size = fileInfo.nFileSizeLow;
    if (fileInfo.nFileSizeHigh)
    {
        info->file_size += ((uint64_t)fileInfo.nFileSizeHigh << 32);
    }
}

void skr_dstorage_close_file(SkrDStorageInstanceId inst, SkrDStorageFileHandle file)
{
    IDStorageFile* pFile = (IDStorageFile*)file;
    pFile->Close();
    pFile->Release();
}

SkrDStorageEventId skr_dstorage_queue_create_event(SkrDStorageQueueId queue)
{
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    return Q->pInstance->pEventPool->Allocate();
}

bool skr_dstorage_event_test(SkrDStorageEventId event)
{
    return event->test();
}

void skr_dstorage_queue_free_event(SkrDStorageQueueId queue, SkrDStorageEventId event)
{
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    SKR_ASSERT(event->test() || !event->get_enqueued() && "You must wait for the event before freeing it!");
    Q->pInstance->pEventPool->Deallocate(event);
}

void skr_dstorage_queue_submit(SkrDStorageQueueId queue, SkrDStorageEventId event)
{
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    event->enqueue_status(Q->pQueue);
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    skr_dstorage_queue_trace_submit(queue);
#endif
    Q->pQueue->Submit();
}

void skr_dstorage_enqueue_request(SkrDStorageQueueId queue, const SkrDStorageIODescriptor* desc)
{
    SkrZoneScopedN("EnqueueDStorage(Memory)");

    DSTORAGE_REQUEST request = {};
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_MEMORY;
    if (desc->compression == 0 || desc->compression >= SKR_DSTORAGE_COMPRESSION_CUSTOM)
    {
        request.Options.CompressionFormat = (DSTORAGE_COMPRESSION_FORMAT)desc->compression;
    }
    if (desc->source_type == SKR_DSTORAGE_SOURCE_FILE)
    {
        SKR_ASSERT(Q->source_type == DSTORAGE_REQUEST_SOURCE_FILE);

        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
        request.Source.File.Source = (IDStorageFile*)desc->source_file.file;
        request.Source.File.Offset = desc->source_file.offset;
        request.Source.File.Size = (uint32_t)desc->source_file.size;
    }
    else if (desc->source_type == SKR_DSTORAGE_SOURCE_MEMORY)
    {
        SKR_ASSERT(Q->source_type == DSTORAGE_REQUEST_SOURCE_MEMORY);

        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
        request.Source.Memory.Source = desc->source_memory.bytes;
        request.Source.Memory.Size = (uint32_t)desc->source_memory.bytes_size;
    }
    request.Destination.Memory.Size = (uint32_t)desc->uncompressed_size;
    request.Destination.Memory.Buffer = desc->destination;
    request.UncompressedSize = (uint32_t)desc->uncompressed_size;
    Q->pQueue->EnqueueRequest(&request);
    if (auto event = desc->event)
        event->enqueue_status(Q->pQueue);
}

#ifdef TRACY_PROFILE_DIRECT_STORAGE
void skr_dstorage_queue_trace_submit(SkrDStorageQueueId queue)
{
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    static uint64_t submit_index = 0;
    HANDLE event_handle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
    DStorageQueueWindows::ProfileTracer* tracer = nullptr;
    {
        SMutexLock profile_lock(Q->profile_mutex);
        for (auto&& _tracer : Q->profile_tracers) // find and use an existed & finished tracer
        {
            if (skr_atomicu32_load_acquire(&_tracer->finished))
            {
                tracer = _tracer;
                skr_destroy_thread(tracer->thread_handle);
                tracer->fence->SetEventOnCompletion(tracer->fence_value++, event_handle);
                tracer->finished = 0;
                break;
            }
        }
    }
    if (tracer == nullptr)
    {
        tracer = SkrNew<DStorageQueueWindows::ProfileTracer>();
        tracer->fence_value = 1;
        Q->pDxDevice->CreateFence(0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tracer->fence));
        tracer->fence->SetEventOnCompletion(tracer->fence_value, event_handle);
        {
            SMutexLock profile_lock(Q->profile_mutex);
            Q->profile_tracers.emplace_back(tracer);
        }
    }
    Q->pQueue->EnqueueSignal(tracer->fence, tracer->fence_value);
    tracer->Q = Q;
    tracer->fence_event = event_handle;
    tracer->submit_index = submit_index++;
    if (Q->device)
        tracer->name = skr::format(u8"DirectStorageQueueSubmit(VRAM)-{}", tracer->submit_index);
    else
        tracer->name = skr::format(u8"DirectStorageQueueSubmit(RAM)-{}", tracer->submit_index);
    tracer->desc.pFunc = +[](void* arg){
        auto tracer = (DStorageQueueWindows::ProfileTracer*)arg;
        auto Q = tracer->Q;
        const auto event_handle = tracer->fence_event;
        SkrFiberEnter(tracer->name.c_str());
        if (Q->source_type == DSTORAGE_REQUEST_SOURCE_FILE)
        {
            SkrZoneScopedN("Working(File)");
            WaitForSingleObject(event_handle, INFINITE);
        }
        else if (Q->source_type == DSTORAGE_REQUEST_SOURCE_MEMORY)
        {
            SkrZoneScopedN("Working(Memory)");
            WaitForSingleObject(event_handle, INFINITE);
        }
        else
        {
            WaitForSingleObject(event_handle, INFINITE);
        }
        SkrFiberLeave;
        CloseHandle(event_handle);
        skr_atomicu32_store_release(&tracer->finished, 1);
    };
    tracer->desc.pData = tracer;
    skr_init_thread(&tracer->desc, &tracer->thread_handle);
    submit_index++;
}
#endif