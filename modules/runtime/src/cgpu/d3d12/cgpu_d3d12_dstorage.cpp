#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"
#include "../common/common_utils.h"

#include "platform/shared_library.hpp"
#include "dstorage.h"
#include "EASTL/vector_map.h"
#include "EASTL/algorithm.h"

#define TRACY_PROFILE_DIRECT_STORAGE
#include "tracy/Tracy.hpp"

struct CGPUDStorageQueueD3D12 : public CGPUDStorageQueue {
    IDStorageQueue* pQueue;
    IDStorageFactory* pFactory;
    uint64_t max_size;
    DSTORAGE_REQUEST_SOURCE_TYPE source_type;
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    SMutex profile_mutex;
    struct ProfileTracer {
        CGPUDStorageQueueD3D12* Q;
        ID3D12Fence* fence;
        SThreadDesc desc;
        SThreadHandle thread_handle;
        HANDLE fence_event;
        uint64_t submit_index;
        uint32_t fence_value = 0;
        SAtomic32 finished;
    };
    eastl::vector<ProfileTracer*> profile_tracers;
#endif

    ~CGPUDStorageQueueD3D12() SKR_NOEXCEPT {
#ifdef TRACY_PROFILE_DIRECT_STORAGE
        for (auto&& tracer : profile_tracers)
        {
            if (!skr_atomic32_load_acquire(&tracer->finished))
            {
                skr_join_thread(tracer->thread_handle);
            }
            skr_destroy_thread(tracer->thread_handle);
            tracer->fence->Release();
            SkrDelete(tracer);
        }
#endif
    }
};


#define CGPU_DSTORAGE_SINGLETON_NAME "CGPUDStorageSingleton"

struct CGPUDStorageSingleton
{
    static CGPUDStorageSingleton* Get(CGPUInstanceId instance)
    {
        auto _this = (CGPUDStorageSingleton*)cgpu_runtime_table_try_get_custom_data(instance->runtime_table, CGPU_DSTORAGE_SINGLETON_NAME);
        if (!_this)
        {
            _this = SkrNew<CGPUDStorageSingleton>();
            {
                _this->dstorage_core.load("dstoragecore.dll");
                _this->dstorage_library.load("dstorage.dll");
                if (!_this->dstorage_core.isLoaded() || !_this->dstorage_library.isLoaded())
                {
                    if (!_this->dstorage_core.isLoaded()) SKR_LOG_TRACE("dstoragecore.dll not found, direct storage is disabled");
                    if (!_this->dstorage_library.isLoaded()) SKR_LOG_TRACE("dstorage.dll not found, direct storage is disabled");
                    _this->dstorage_dll_dont_exist = true;
                }
                else
                {
                    SKR_LOG_TRACE("dstorage.dll loaded");

                    auto pfn_get_factory = SKR_SHARED_LIB_LOAD_API(_this->dstorage_library, DStorageGetFactory);
                    if (!pfn_get_factory) return nullptr;
                    
                    if (!SUCCEEDED(pfn_get_factory(IID_PPV_ARGS(&_this->pFactory))))
                    {
                        SKR_LOG_ERROR("Failed to get DStorage factory!");
                        return nullptr;
                    }
                }
            }
            cgpu_runtime_table_add_custom_data(instance->runtime_table, CGPU_DSTORAGE_SINGLETON_NAME, _this);
            auto sweep = +[](void* usrdata)
            {
                auto _this = (CGPUDStorageSingleton*)usrdata;
                SkrDelete(_this);
            };
            cgpu_runtime_table_add_sweep_callback(instance->runtime_table, CGPU_DSTORAGE_SINGLETON_NAME, sweep, _this);
        }
        return _this->dstorage_dll_dont_exist ? nullptr : _this;
    }

    ~CGPUDStorageSingleton()
    {
        if (pFactory) pFactory->Release();
        if (dstorage_core.isLoaded()) dstorage_core.unload();
        if (dstorage_library.isLoaded()) dstorage_library.unload();
        
        SKR_LOG_TRACE("Direct Storage unloaded");
    }

    IDStorageFactory* pFactory = nullptr;
    skr::SharedLibrary dstorage_library;
    skr::SharedLibrary dstorage_core;
    bool dstorage_dll_dont_exist = false;
    eastl::vector_map<CGPUDeviceId, ECGPUDStorageAvailability> availability_map = {};
    uint64_t sDirectStorageStagingBufferSize = DSTORAGE_STAGING_BUFFER_SIZE_32MB;
};

inline static IDStorageFactory* GetDStorageFactory(CGPUInstanceId instance)
{
    return CGPUDStorageSingleton::Get(instance) ? CGPUDStorageSingleton::Get(instance)->pFactory : nullptr;
}

ECGPUDStorageAvailability cgpu_query_dstorage_availability_d3d12(CGPUDeviceId device)
{
    auto instance = device->adapter->instance;
    auto _this = CGPUDStorageSingleton::Get(instance);
    if (!_this) return CGPU_DSTORAGE_AVAILABILITY_NONE;
    
    auto res = _this->availability_map.find(device);
    if (res == _this->availability_map.end())
    {
        if (!GetDStorageFactory(instance))
        {
            _this->availability_map[device] = CGPU_DSTORAGE_AVAILABILITY_NONE;
        }
        else
        {
            _this->availability_map[device] = CGPU_DSTORAGE_AVAILABILITY_HARDWARE;
        }
    }
    return _this->availability_map[device];
}

CGPUDStorageQueueId cgpu_create_dstorage_queue_d3d12(CGPUDeviceId device, const CGPUDStorageQueueDescriptor* desc)
{
    auto _this = CGPUDStorageSingleton::Get(device->adapter->instance);
    if (!_this) return nullptr;

    CGPUDStorageQueueD3D12* Q = SkrNew<CGPUDStorageQueueD3D12>();
    auto Device = (CGPUDevice_D3D12*)device;
    DSTORAGE_QUEUE_DESC queueDesc{};
    queueDesc.Capacity = desc->capacity;
    queueDesc.Priority = (DSTORAGE_PRIORITY)desc->priority;
    Q->source_type = queueDesc.SourceType = (DSTORAGE_REQUEST_SOURCE_TYPE)desc->source;
    queueDesc.Name = desc->name;
    if(Device) queueDesc.Device = Device->pDxDevice;
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
    Q->device = device;
    return Q;
}

#include <filesystem>

CGPUDStorageFileHandle cgpu_dstorage_open_file_d3d12(CGPUDStorageQueueId queue, const char* abs_path)
{
    IDStorageFile* pFile = nullptr;
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;
    auto absPath = std::filesystem::path(abs_path);
    Q->pFactory->OpenFile(absPath.c_str(), IID_PPV_ARGS(&pFile));
    return (CGPUDStorageFileHandle)pFile;
}

void cgpu_dstorage_query_file_info_d3d12(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file, CGPUDStorageFileInfo* info)
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

void cgpu_dstorage_enqueue_buffer_request_d3d12(CGPUDStorageQueueId queue, const CGPUDStorageBufferIODescriptor* desc)
{
    ZoneScopedN("D3D12EnqueueDStorage(Buffer)");

    DSTORAGE_REQUEST request = {};
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;
    CGPUBuffer_D3D12* pBuffer = (CGPUBuffer_D3D12*)desc->buffer;
    request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;
    if (desc->compression == 0 || desc->compression >= CGPU_DSTORAGE_COMPRESSION_CUSTOM)
    {
        request.Options.CompressionFormat = (DSTORAGE_COMPRESSION_FORMAT)desc->compression;
    }
    if (desc->source_type == CGPU_DSTORAGE_SOURCE_FILE)
    {
        SKR_ASSERT(Q->source_type == DSTORAGE_REQUEST_SOURCE_FILE);

        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
        request.Source.File.Source = (IDStorageFile*)desc->source_file.file;
        request.Source.File.Offset = desc->source_file.offset;
        request.Source.File.Size = (uint32_t)desc->source_file.size;
    }
    else if (desc->source_type == CGPU_DSTORAGE_SOURCE_MEMORY)
    {
        SKR_ASSERT(Q->source_type == DSTORAGE_REQUEST_SOURCE_MEMORY);

        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
        request.Source.Memory.Source = desc->source_memory.bytes;
        request.Source.Memory.Size = (uint32_t)desc->source_memory.bytes_size;
    }
    request.Destination.Buffer.Resource = pBuffer->pDxResource;
    request.Destination.Buffer.Offset = desc->offset;
    request.Destination.Buffer.Size = (uint32_t)desc->uncompressed_size;
    request.UncompressedSize = (uint32_t)desc->uncompressed_size;
    Q->pQueue->EnqueueRequest(&request);
    if (desc->fence)
    {
        CGPUFence_D3D12* F = (CGPUFence_D3D12*)desc->fence;
        Q->pQueue->EnqueueSignal(F->pDxFence, F->mFenceValue++);
    }
}

void cgpu_dstorage_enqueue_texture_request_d3d12(CGPUDStorageQueueId queue, const CGPUDStorageTextureIODescriptor* desc)
{
    ZoneScopedN("D3D12EnqueueDStorage(Texture)");

    DSTORAGE_REQUEST request = {};
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;
    CGPUTexture_D3D12* pTexture = (CGPUTexture_D3D12*)desc->texture;
    request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_TEXTURE_REGION;
    if (desc->compression == 0 || desc->compression >= CGPU_DSTORAGE_COMPRESSION_CUSTOM)
    {
        request.Options.CompressionFormat = (DSTORAGE_COMPRESSION_FORMAT)desc->compression;
    }
    if (desc->source_type == CGPU_DSTORAGE_SOURCE_FILE)
    {
        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
        request.Source.File.Source = (IDStorageFile*)desc->source_file.file;
        request.Source.File.Offset = desc->source_file.offset;
        request.Source.File.Size = (uint32_t)desc->source_file.size;
    }
    else if (desc->source_type == CGPU_DSTORAGE_SOURCE_MEMORY)
    {
        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
        request.Source.Memory.Source = desc->source_memory.bytes;
        request.Source.Memory.Size = (uint32_t)desc->source_memory.bytes_size;
    }
    request.Destination.Texture.Resource = pTexture->pDxResource;
    // TODO: Support mip & array
    request.Destination.Texture.SubresourceIndex = 0; // mipIndex + (arrayIndex * textureMetaData.mipLevels);
    request.Destination.Texture.Region = { 0, 0, 0, 0, 0, 0 };
    request.Destination.Texture.Region.right = desc->width;
    request.Destination.Texture.Region.bottom = desc->height;
    request.Destination.Texture.Region.back = desc->depth;
    request.UncompressedSize = (uint32_t)desc->uncompressed_size;
    SKR_ASSERT(desc->uncompressed_size <= Q->max_size);
    Q->pQueue->EnqueueRequest(&request);
    if (desc->fence)
    {
        CGPUFence_D3D12* F = (CGPUFence_D3D12*)desc->fence;
        Q->pQueue->EnqueueSignal(F->pDxFence, F->mFenceValue++);
    }
}

void cgpu_dstorage_queue_submit_d3d12(CGPUDStorageQueueId queue, CGPUFenceId fence)
{
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;
    CGPUFence_D3D12* F = (CGPUFence_D3D12*)fence;

#ifdef TRACY_PROFILE_DIRECT_STORAGE
    {
        static uint64_t submit_index = 0;
        auto D = (CGPUDevice_D3D12*)F->super.device;
        HANDLE event_handle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        CGPUDStorageQueueD3D12::ProfileTracer* tracer = nullptr;
        {
            SMutexLock profile_lock(Q->profile_mutex);
            for (auto&& _tracer : Q->profile_tracers)
            {
                if (skr_atomic32_load_acquire(&_tracer->finished))
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
            tracer = SkrNew<CGPUDStorageQueueD3D12::ProfileTracer>();
            tracer->fence_value = 1;
            D->pDxDevice->CreateFence(0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tracer->fence));
            tracer->fence->SetEventOnCompletion(tracer->fence_value, event_handle);
            {
                SMutexLock profile_lock(Q->profile_mutex);
                Q->profile_tracers.emplace_back(tracer);
            }
        }
        Q->pQueue->EnqueueSignal(tracer->fence, tracer->fence_value);
        tracer->fence_event = event_handle;
        tracer->submit_index = submit_index++;
        tracer->Q = Q;
        tracer->desc.pData = tracer;
        tracer->desc.pFunc = +[](void* arg){
            auto tracer = (CGPUDStorageQueueD3D12::ProfileTracer*)arg;
            auto Q = tracer->Q;
            const auto event_handle = tracer->fence_event;
            eastl::string name = "DirectStorageQueueSubmit-";
            name += eastl::to_string(tracer->submit_index);
            TracyFiberEnter(name.c_str());
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
            skr_atomic32_store_release(&tracer->finished, 1);
        };
        skr_init_thread(&tracer->desc, &tracer->thread_handle);
        submit_index++;
    }
#endif
    Q->pQueue->EnqueueSignal(F->pDxFence, F->mFenceValue++);
    Q->pQueue->Submit();
}

void cgpu_dstorage_close_file_d3d12(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file)
{
    IDStorageFile* pFile = (IDStorageFile*)file;
    pFile->Close();
    pFile->Release();
}

void cgpu_free_dstorage_queue_d3d12(CGPUDStorageQueueId queue)
{
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;

    if (Q->pQueue)
    {
        // TODO: WaitErrorEvent & Handle errors 
        DSTORAGE_ERROR_RECORD record = {};
        Q->pQueue->RetrieveErrorRecord(&record);

#ifdef TRACY_PROFILE_DIRECT_STORAGE
        skr_destroy_mutex(&Q->profile_mutex);
#endif

        Q->pQueue->Release();
    }

    SkrDelete(Q);
}

#include <EASTL/string.h>
#include <EASTL/vector_map.h>
#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "platform/memory.h"
#include "platform/thread.h"
#include "cgpu/extensions/dstorage_windows.h"
#include "cgpu/io.hpp"

static void CALLBACK __decompressThreadPoolTask_DirectStorage(
    TP_CALLBACK_INSTANCE*,
    void*,
    TP_WAIT* wait,
    TP_WAIT_RESULT);

static void CALLBACK __decompressThreadTask_DirectStorage(void*);

struct skr_win_dstorage_decompress_service_t
{
    const bool use_thread_pool = true;
    skr_win_dstorage_decompress_service_t(IDStorageCustomDecompressionQueue* queue)
        : decompress_queue(queue)
    {
        skr_atomic32_store_release(&thread_running, 1);
        event_handle = queue->GetEvent();
        if (use_thread_pool)
        {
            thread_pool_wait = CreateThreadpoolWait(__decompressThreadPoolTask_DirectStorage, this, nullptr);
            SetThreadpoolWait(thread_pool_wait, event_handle, nullptr);
        }
        else 
        {
            thread_desc.pData = this;
            thread_desc.pFunc = &__decompressThreadTask_DirectStorage;
            skr_init_thread(&thread_desc, &thread_handle);
        }
    }
    ~skr_win_dstorage_decompress_service_t  () SKR_NOEXCEPT
    {
        skr_atomic32_store_release(&thread_running, 0);
        if (use_thread_pool)
        {
            CloseThreadpoolWait(thread_pool_wait);
        }
        else
        {
            SetEvent(event_handle);
            skr_join_thread(thread_handle);
            skr_destroy_thread(thread_handle);
        }
        CloseHandle(event_handle);
        decompress_queue->Release();
    }

    // queue items
    struct DecompressionResolver {
        skr_win_dstorage_decompress_callback_t callback = nullptr;
        void* user_data = nullptr;
    };
    IDStorageCustomDecompressionQueue* decompress_queue = nullptr;
    HANDLE event_handle = nullptr;
    eastl::vector_map<CGPUDStorageCompression, DecompressionResolver> resolvers;
    // threadpool items
    TP_WAIT* thread_pool_wait = nullptr;
    // thread items
    SThreadDesc thread_desc;
    SThreadHandle thread_handle = nullptr;
    SAtomic32 thread_running;
};

static void __decompressTask_DirectStorage(skr_win_dstorage_decompress_service_id service)
{
    auto running = skr_atomic32_load_acquire(&service->thread_running);
    while (running)
    {
        DSTORAGE_CUSTOM_DECOMPRESSION_REQUEST requests[64];
        uint32_t numRequests = 0;

        // Pull off a batch of requests to process in this loop.
        service->decompress_queue->GetRequests(_countof(requests), requests, &numRequests);

        if (numRequests == 0)
            break;

        {
            ZoneScopedN("DirectStorageDecompress");
            for (uint32_t i = 0; i < numRequests; ++i)
            {
                auto resolver = service->resolvers.find(requests[i].CompressionFormat);
                if (resolver != service->resolvers.end())
                {
                    auto skrRequest = make_zeroed<skr_win_dstorage_decompress_request_t>();
                    skrRequest.id = requests[i].Id;
                    skrRequest.compression = requests[i].CompressionFormat;
                    skrRequest.flags = requests[i].Flags;
                    skrRequest.src_size = requests[i].SrcSize;
                    skrRequest.src_buffer = requests[i].SrcBuffer;
                    skrRequest.dst_size = requests[i].DstSize;
                    skrRequest.dst_buffer = requests[i].DstBuffer;
                    auto result = resolver->second.callback(&skrRequest, resolver->second.user_data);
                    DSTORAGE_CUSTOM_DECOMPRESSION_RESULT failResult = {};
                    failResult.Result = result;
                    failResult.Id = requests[i].Id;
                    service->decompress_queue->SetRequestResults(1, &failResult);
                }
                else
                {
                    SKR_LOG_WARN("Unable to find decompression resolver for format %d\n", requests[i].CompressionFormat);
                    DSTORAGE_CUSTOM_DECOMPRESSION_RESULT failResult = {};
                    failResult.Result = S_FALSE;
                    failResult.Id = requests[i].Id;
                    service->decompress_queue->SetRequestResults(1, &failResult);
                }
            }
        }
    }
}

static void CALLBACK __decompressThreadPoolTask_DirectStorage(
    TP_CALLBACK_INSTANCE*,
    void* data,
    TP_WAIT* wait,
    TP_WAIT_RESULT)
{
#ifdef TRACY_ENABLE
    auto thread_id = skr_current_thread_id();
    const eastl::string name = "DirectStorageDecompressThread(Pooled)-";
    auto indexed_name = name + eastl::to_string(thread_id);
    tracy::SetThreadName(indexed_name.c_str());
#endif

    auto service = (skr_win_dstorage_decompress_service_id)data;
    auto oldPriority = GetThreadPriority(GetCurrentThread());
    {
        ZoneScopedN("DirectStorageDecompress");
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

        __decompressTask_DirectStorage(service);
    }
    
#ifdef TRACY_ENABLE
    tracy::SetThreadName("");
#endif

    // Restore the original thread's priority back to its original setting.
    SetThreadPriority(GetCurrentThread(), oldPriority);
    // Re-register the custom decompression queue event with this callback
    // to be called when the next decompression requests become available for processing.
    SetThreadpoolWait(wait, service->event_handle, nullptr);
}

static void CALLBACK __decompressThreadTask_DirectStorage(void* data)
{
#ifdef TRACY_ENABLE
    tracy::SetThreadName("DirectStorageDecompressThread");
#endif
    auto service = (skr_win_dstorage_decompress_service_id)data;
    while (skr_atomic32_load_acquire(&service->thread_running))
    {
        {
            ZoneScopedNC("DirectStorageDecompressWait", tracy::Color::Gray43);
            WaitForSingleObject(service->event_handle, INFINITE);
        }
        __decompressTask_DirectStorage(service);
    } 
}

void cgpu_win_dstorage_set_staging_buffer_size(CGPUInstanceId instance, uint64_t size)
{
    auto _this = CGPUDStorageSingleton::Get(instance);
    if (!_this) return;
    if (!_this->pFactory) return;
    _this->pFactory->SetStagingBufferSize((uint32_t)size);
    _this->sDirectStorageStagingBufferSize = size;
}

skr_win_dstorage_decompress_service_id cgpu_win_create_decompress_service(CGPUInstanceId instance)
{
    auto _this = CGPUDStorageSingleton::Get(instance);
    if (!_this) return nullptr;
    if (!_this->pFactory) return nullptr;

    IDStorageFactory* pFactory = _this->pFactory;
    if (!pFactory) return nullptr;
    IDStorageCustomDecompressionQueue* pCompressionQueue = nullptr;
    if (!SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&pCompressionQueue))))
    {
        return nullptr;
    }
    auto service = SkrNew<skr_win_dstorage_decompress_service_t>(pCompressionQueue);
    SKR_LOG_TRACE("Created decompress service");
    return service;
}

bool cgpu_win_decompress_service_register_callback(skr_win_dstorage_decompress_service_id service, 
    CGPUDStorageCompression compression, skr_win_dstorage_decompress_callback_t callback, void* user_data)
{
    const auto registered = (service->resolvers.find(compression) != service->resolvers.end());
    SKR_ASSERT(!registered && "Callback already registered for this compression");
    if (registered) return false;
    SKR_ASSERT(callback && "Callback must be valid");
    service->resolvers[compression] = { callback, user_data };
    return true;
}

void cgpu_win_free_decompress_service(skr_win_dstorage_decompress_service_id service)
{
    SKR_ASSERT(service && "Invalid service");
    service->decompress_queue->Release();
    SkrDelete(service);
    SKR_LOG_TRACE("Deleted decompress service");
}