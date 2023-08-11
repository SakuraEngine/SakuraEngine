#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "../common/common_utils.h"

#include "platform/windows/windows_dstorage.hpp"

#include "EASTL/vector_map.h"

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
            return _instance->initialize_failed ? nullptr : _this;
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
    SkrDStorageQueueDescriptor desc2 = *desc;
    desc2.gpu_device = device;
    return skr_create_dstorage_queue(&desc2);
}

void cgpu_free_dstorage_queue_d3d12(CGPUDStorageQueueId queue)
{
    skr_free_dstorage_queue(queue);
}

#include <filesystem>

CGPUDStorageFileHandle cgpu_dstorage_open_file_d3d12(CGPUDStorageQueueId queue, const char8_t* abs_path)
{
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;
    return skr_dstorage_open_file(Q->pInstance, abs_path);
}

void cgpu_dstorage_query_file_info_d3d12(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file, CGPUDStorageFileInfo* info)
{
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;
    return skr_dstorage_query_file_info(Q->pInstance, file, info);
}

void cgpu_dstorage_enqueue_buffer_request_d3d12(CGPUDStorageQueueId queue, const CGPUDStorageBufferIODescriptor* desc)
{
    SkrZoneScopedN("D3D12EnqueueDStorage(Buffer)");

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
    SkrZoneScopedN("D3D12EnqueueDStorage(Texture)");

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
    Q->pQueue->EnqueueSignal(F->pDxFence, F->mFenceValue++);
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    skr_dstorage_queue_trace_submit(queue);
#endif
    Q->pQueue->Submit();
}

void cgpu_dstorage_close_file_d3d12(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file)
{
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)queue;
    skr_dstorage_close_file(Q->pInstance, file);
}