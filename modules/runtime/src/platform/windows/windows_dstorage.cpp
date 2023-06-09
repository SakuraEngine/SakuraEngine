#include "platform/windows/windows_dstorage.hpp"
#include "cgpu/extensions/cgpu_d3d12_exts.h"
#include "misc/make_zeroed.hpp"
#include "platform/win/misc.h"

#include "EASTL/vector_map.h"
#include "EASTL/algorithm.h"

SkrWindowsDStorageInstance* SkrWindowsDStorageInstance::_this = nullptr;

SkrWindowsDStorageInstance* SkrWindowsDStorageInstance::Get()
{
    return _this;
}

SkrWindowsDStorageInstance* SkrWindowsDStorageInstance::Initialize(const SkrDStorageConfig& cfg)
{
    const bool wine = skr_win_is_wine();
    if (wine)
        _this->dstorage_dll_dont_exist = true;
    else if (!_this)
    {
        _this = SkrNew<SkrWindowsDStorageInstance>();
        {
            _this->dstorage_core.load(u8"dstoragecore.dll");
            _this->dstorage_library.load(u8"dstorage.dll");
            if (!_this->dstorage_core.isLoaded() || !_this->dstorage_library.isLoaded())
            {
                if (!_this->dstorage_core.isLoaded()) SKR_LOG_TRACE("dstoragecore.dll not found, direct storage is disabled");
                if (!_this->dstorage_library.isLoaded()) SKR_LOG_TRACE("dstorage.dll not found, direct storage is disabled");
                _this->dstorage_dll_dont_exist = true;
            }
            else
            {
                SKR_LOG_TRACE("dstorage.dll loaded");

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
                    SKR_LOG_ERROR("Failed to set DStorage config!");
                    return nullptr;
                }

                auto pfn_get_factory = SKR_SHARED_LIB_LOAD_API(_this->dstorage_library, DStorageGetFactory);
                if (!pfn_get_factory) return nullptr;
                if (!SUCCEEDED(pfn_get_factory(IID_PPV_ARGS(&_this->pFactory))))
                {
                    SKR_LOG_ERROR("Failed to get DStorage factory!");
                    return nullptr;
                }

                // Create WRAP Device
                D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_this->pWarpDevice));
            }
        }
    }
    return _this->dstorage_dll_dont_exist ? nullptr : _this;
}


SkrWindowsDStorageInstance::~SkrWindowsDStorageInstance()
{
    if (pFactory) pFactory->Release();
    if (pWarpDevice) pWarpDevice->Release();
    if (dstorage_core.isLoaded()) dstorage_core.unload();
    if (dstorage_library.isLoaded()) dstorage_library.unload();
    
    SKR_LOG_TRACE("Direct Storage unloaded");
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
    auto inst = SkrWindowsDStorageInstance::Get();
    return inst ? SKR_DSTORAGE_AVAILABILITY_HARDWARE : SKR_DSTORAGE_AVAILABILITY_NONE;
}

void skr_free_dstorage_instance(SkrDStorageInstanceId inst)
{
    SkrDelete(inst);
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
        SKR_LOG_ERROR("Failed to create DStorage queue!");
        SkrDelete(Q);
        return nullptr;
    }
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    skr_init_mutex_recursive(&Q->profile_mutex);
#endif
    Q->max_size = _this->sDirectStorageStagingBufferSize;
    Q->pFactory = pFactory;
    Q->pDxDevice = queueDesc.Device ? queueDesc.Device : _this->pWarpDevice;
    Q->pFactory->AddRef();
    Q->pDxDevice->AddRef();
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
        Q->pDxDevice->Release();
        Q->pFactory->Release();
    }
    SkrDelete(Q);
}

#include <filesystem>

SkrDStorageFileHandle skr_dstorage_open_file(SkrDStorageInstanceId inst, const char* abs_path)
{
    IDStorageFile* pFile = nullptr;
    auto I = (SkrWindowsDStorageInstance*)inst;
    auto absPath = std::filesystem::path(abs_path);
    I->pFactory->OpenFile(absPath.c_str(), IID_PPV_ARGS(&pFile));
    return (SkrDStorageFileHandle)pFile;
}

void skr_dstorage_query_file_info(SkrDStorageInstanceId inst, SkrDStorageFileHandle file, SkrDStorageFileInfo* info)
{
    BY_HANDLE_FILE_INFORMATION fileInfo;
    IDStorageFile* pFile = (IDStorageFile*)file;
    if (!SUCCEEDED(pFile->GetFileInformation(&fileInfo)))
    {
        SKR_LOG_ERROR("Failed to get DStorage file info!");
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

struct SkrDStorageEvent
{
    SkrDStorageEvent(ID3D12Fence* pFence) : pFence(pFence), mFenceValue(1) {}
    ID3D12Fence* pFence = nullptr;
    uint64_t mFenceValue = 1;
};

SkrDStorageEventId skr_dstorage_queue_create_event(SkrDStorageQueueId queue)
{
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    ID3D12Fence* pFence = nullptr;
    Q->pDxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
    return SkrNew<SkrDStorageEvent>(pFence);
}

bool skr_dstorage_event_test(SkrDStorageEventId event)
{
    const auto Value = event->pFence->GetCompletedValue();
    if (Value < event->mFenceValue - 1)
        return false;
    else
        return true;
}

void skr_dstorage_queue_free_event(SkrDStorageQueueId queue, SkrDStorageEventId event)
{
    event->pFence->Release();
}

void skr_dstorage_queue_submit(SkrDStorageQueueId queue, SkrDStorageEventId event)
{
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    Q->pQueue->EnqueueSignal(event->pFence, event->mFenceValue++);
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    skr_dstorage_queue_trace_submit(queue);
#endif
    Q->pQueue->Submit();
}

void skr_dstorage_enqueue_request(SkrDStorageQueueId queue, const SkrDStorageIODescriptor* desc)
{
    ZoneScopedN("EnqueueDStorage(Memory)");

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
        Q->pQueue->EnqueueSignal(event->pFence, event->mFenceValue++);
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
        TracyFiberEnter(tracer->name.c_str());
        if (Q->source_type == DSTORAGE_REQUEST_SOURCE_FILE)
        {
            ZoneScopedN("Working(File)");
            WaitForSingleObject(event_handle, INFINITE);
        }
        else if (Q->source_type == DSTORAGE_REQUEST_SOURCE_MEMORY)
        {
            ZoneScopedN("Working(Memory)");
            WaitForSingleObject(event_handle, INFINITE);
        }
        else
        {
            WaitForSingleObject(event_handle, INFINITE);
        }
        TracyFiberLeave;
        CloseHandle(event_handle);
        skr_atomicu32_store_release(&tracer->finished, 1);
    };
    tracer->desc.pData = tracer;
    skr_init_thread(&tracer->desc, &tracer->thread_handle);
    submit_index++;
}
#endif