#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"
#include "../common/common_utils.h"

#if !defined(XBOX)
    #pragma comment(lib, "dstorage.lib")
#endif

#include "dstorage.h"
#include "EASTL/vector_map.h"

struct CGPUDStorageQueueD3D12 : public CGPUDStorageQueue {
    IDStorageQueue* pQueue;
    IDStorageFactory* pFactory;
    SMutex request_mutex;
};

thread_local static eastl::vector_map<CGPUDeviceId, ECGPUDStorageAvailability> availability_map = {};
ECGPUDStorageAvailability cgpu_query_dstorage_availability_d3d12(CGPUDeviceId device)
{
    auto res = availability_map.find(device);
    if (res == availability_map.end())
    {
        IDStorageFactory* pFactory = nullptr;
        if (!SUCCEEDED(DStorageGetFactory(IID_PPV_ARGS(&pFactory))))
        {
            availability_map[device] = CGPU_DSTORAGE_AVAILABILITY_NONE;
        }
        else
        {
            availability_map[device] = CGPU_DSTORAGE_AVAILABILITY_HARDWARE;
            pFactory->Release();
        }
    }
    return availability_map[device];
}

CGPUDStorageQueueId cgpu_create_dstorage_queue_d3d12(CGPUDeviceId device, const CGPUDStorageQueueDescriptor* desc)
{
    CGPUDStorageQueueD3D12* Q = SkrNew<CGPUDStorageQueueD3D12>();
    auto Device = (CGPUDevice_D3D12*)device;
    DSTORAGE_QUEUE_DESC queueDesc{};
    queueDesc.Capacity = desc->capacity;
    queueDesc.Priority = (DSTORAGE_PRIORITY)desc->priority;
    queueDesc.SourceType = (DSTORAGE_REQUEST_SOURCE_TYPE)desc->source;
    queueDesc.Name = desc->name;
    if(Device) queueDesc.Device = Device->pDxDevice;
    IDStorageFactory* pFactory = nullptr;
    if (!SUCCEEDED(DStorageGetFactory(IID_PPV_ARGS(&pFactory))))
    {
        SKR_LOG_ERROR("Failed to get DStorage factory!");
        SkrDelete(Q);
        return nullptr;
    }
    if (!SUCCEEDED(pFactory->CreateQueue(&queueDesc, IID_PPV_ARGS(&Q->pQueue))))
    {
        SKR_LOG_ERROR("Failed to create DStorage queue!");
        SkrDelete(Q);
        return nullptr;
    }
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
    request.Destination.Buffer.Resource = pBuffer->pDxResource;
    request.Destination.Buffer.Offset = desc->offset;
    request.Destination.Buffer.Size = (uint32_t)desc->size;
    request.UncompressedSize = (uint32_t)desc->uncompressed_size;
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
        Q->pFactory->Release();
        Q->pQueue->Release();
    }

    SkrDelete(Q);
}


#include "utils/log.h"
#include "platform/memory.h"
#include "platform/thread.h"
#include "cgpu/extensions/dstorage_windows.h"
#include "cgpu/io.hpp"

static void CALLBACK __decompressThreadTask_DirectStorage(
    TP_CALLBACK_INSTANCE*,
    void*,
    TP_WAIT* wait,
    TP_WAIT_RESULT);

struct skr_win_dstorage_decompress_service_t
{
    skr_win_dstorage_decompress_service_t(IDStorageCustomDecompressionQueue* queue)
        : decompress_queue(queue)
    {
        event_handle = queue->GetEvent();
        thread_pool_wait = CreateThreadpoolWait(__decompressThreadTask_DirectStorage, this, nullptr);
        SetThreadpoolWait(thread_pool_wait, event_handle, nullptr);
    }
    ~skr_win_dstorage_decompress_service_t() SKR_NOEXCEPT
    {
        CloseThreadpoolWait(thread_pool_wait);
        decompress_queue->Release();
        CloseHandle(event_handle);
    }
    // thread items
    IDStorageCustomDecompressionQueue* decompress_queue = nullptr;
    HANDLE event_handle = nullptr;
    TP_WAIT* thread_pool_wait = nullptr;
};

static void CALLBACK __decompressThreadTask_DirectStorage(
    TP_CALLBACK_INSTANCE*,
    void* data,
    TP_WAIT* wait,
    TP_WAIT_RESULT)
{
    auto service = (skr_win_dstorage_decompress_service_id)data;
    auto oldPriority = GetThreadPriority(GetCurrentThread());
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    while (true)
    {
        DSTORAGE_CUSTOM_DECOMPRESSION_REQUEST requests[64];
        uint32_t numRequests = 0;

        // Pull off a batch of requests to process in this loop.
        service->decompress_queue->GetRequests(_countof(requests), requests, &numRequests);

        if (numRequests == 0)
            break;

        for (uint32_t i = 0; i < numRequests; ++i)
        {
            // ScheduleDecompression(requests[i]);
        }
    }
    
    // Restore the original thread's priority back to its original setting.
    SetThreadPriority(GetCurrentThread(), oldPriority);
    // Re-register the custom decompression queue event with this callback
    // to be called when the next decompression requests become available for processing.
    SetThreadpoolWait(wait, service->event_handle, nullptr);
}

skr_win_dstorage_decompress_service_id cgpu_win_create_decompress_service(CGPUDStorageQueueId dsqueue)
{
    CGPUDStorageQueueD3D12* Q = (CGPUDStorageQueueD3D12*)dsqueue;
    IDStorageFactory* pFactory = Q->pFactory;
    IDStorageCustomDecompressionQueue* pCompressionQueue = nullptr;
    if (!SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&pCompressionQueue))))
    {
        return nullptr;
    }
    auto service = SkrNew<skr_win_dstorage_decompress_service_t>(pCompressionQueue);
    SKR_LOG_DEBUG("Created decompress service");
    return service;
}

void cgpu_win_free_decompress_service(skr_win_dstorage_decompress_service_id service)
{
    SKR_ASSERT(service && "Invalid service");
    SkrDelete(service);
    SKR_LOG_DEBUG("Deleted decompress service");
}