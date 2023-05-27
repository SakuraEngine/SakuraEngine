#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"
#include "../common/common_utils.h"

#include "platform/windows/windows_dstorage.hpp"

#include "EASTL/vector_map.h"
#include "EASTL/algorithm.h"

#define SKR_DSTORAGE_SINGLETON_NAME u8"CGPUDStorageSingleton"

struct CGPUDStorageSingleton
{
    static CGPUDStorageSingleton* Get(CGPUInstanceId instance)
    {
        auto _instance =  (SkrWindowsDStorageInstance*)skr_get_dstorage_instnace();
        if (_instance)
        {
            auto _this = SkrNew<CGPUDStorageSingleton>();
            cgpu_runtime_table_add_custom_data(instance->runtime_table, SKR_DSTORAGE_SINGLETON_NAME, _this);
            auto sweep = +[](void* usrdata)
            {
                auto _this = (CGPUDStorageSingleton*)usrdata;
                SkrDelete(_this);
            };
            cgpu_runtime_table_add_early_sweep_callback(instance->runtime_table, SKR_DSTORAGE_SINGLETON_NAME, sweep, _this);
            return _instance->dstorage_dll_dont_exist ? nullptr : _this;
        }
        return nullptr;
    }
    eastl::vector_map<CGPUDeviceId, ECGPUDStorageAvailability> availability_map = {};
};

inline static IDStorageFactory* GetDStorageFactory(CGPUInstanceId instance)
{
    auto inst = (SkrWindowsDStorageInstance*)skr_get_dstorage_instnace();
    return inst->pFactory;
}

ECGPUDStorageAvailability cgpu_query_dstorage_availability_d3d12(CGPUDeviceId device)
{
    auto instance = device->adapter->instance;
    auto _this = CGPUDStorageSingleton::Get(instance);
    if (!_this) return SKR_DSTORAGE_AVAILABILITY_NONE;
    
    auto res = _this->availability_map.find(device);
    if (res == _this->availability_map.end())
    {
        if (!GetDStorageFactory(instance))
        {
            _this->availability_map[device] = SKR_DSTORAGE_AVAILABILITY_NONE;
        }
        else
        {
            _this->availability_map[device] = SKR_DSTORAGE_AVAILABILITY_HARDWARE;
        }
    }
    return _this->availability_map[device];
}

using CGPUDStorageQueueD3D12 = DStorageQueueWindows;
CGPUDStorageQueueId cgpu_create_dstorage_queue_d3d12(CGPUDeviceId device, const CGPUDStorageQueueDescriptor* desc)
{
    auto _this = (SkrWindowsDStorageInstance*)skr_get_dstorage_instnace();
    if (!_this) return nullptr;

    CGPUDStorageQueueD3D12* Q = SkrNew<CGPUDStorageQueueD3D12>();
    auto Device = (CGPUDevice_D3D12*)device;
    DSTORAGE_QUEUE_DESC queueDesc{};
    queueDesc.Capacity = desc->capacity;
    queueDesc.Priority = (DSTORAGE_PRIORITY)desc->priority;
    Q->source_type = queueDesc.SourceType = (DSTORAGE_REQUEST_SOURCE_TYPE)desc->source;
    queueDesc.Name = (const char*)desc->name;
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

void cgpu_free_dstorage_queue_d3d12(CGPUDStorageQueueId queue)
{
    skr_free_dstorage_queue(queue);
}

#include <filesystem>

CGPUDStorageFileHandle cgpu_dstorage_open_file_d3d12(CGPUDStorageQueueId queue, const char* abs_path)
{
    return skr_dstorage_open_file(queue, abs_path);
}

void cgpu_dstorage_query_file_info_d3d12(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file, CGPUDStorageFileInfo* info)
{
    return skr_dstorage_query_file_info(queue, file, info);
}

void cgpu_dstorage_enqueue_buffer_request_d3d12(CGPUDStorageQueueId queue, const CGPUDStorageBufferIODescriptor* desc)
{
    ZoneScopedN("D3D12EnqueueDStorage(Buffer)");

    DSTORAGE_REQUEST request = {};
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;
    CGPUBuffer_D3D12* pBuffer = (CGPUBuffer_D3D12*)desc->buffer;
    request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;
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
    if (desc->compression == 0 || desc->compression >= SKR_DSTORAGE_COMPRESSION_CUSTOM)
    {
        request.Options.CompressionFormat = (DSTORAGE_COMPRESSION_FORMAT)desc->compression;
    }
    if (desc->source_type == SKR_DSTORAGE_SOURCE_FILE)
    {
        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
        request.Source.File.Source = (IDStorageFile*)desc->source_file.file;
        request.Source.File.Offset = desc->source_file.offset;
        request.Source.File.Size = (uint32_t)desc->source_file.size;
    }
    else if (desc->source_type == SKR_DSTORAGE_SOURCE_MEMORY)
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
            const auto name = skr::format(u8"DirectStorageQueueSubmit-{}", tracer->submit_index);
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
            skr_atomicu32_store_release(&tracer->finished, 1);
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
    skr_dstorage_close_file(queue, file);
}