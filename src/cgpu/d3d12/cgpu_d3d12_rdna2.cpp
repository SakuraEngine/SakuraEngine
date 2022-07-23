#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"
#include "../common/common_utils.h"

void cgpu_render_encoder_set_shading_rate_d3d12(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate)
{
#ifdef D3D12_HEADER_SUPPORT_VRS
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    auto Adapter = (CGPUAdapter_D3D12*)Cmd->super.device->adapter;
    if (Adapter->adapter_detail.support_shading_rate)
    {
        D3D12_SHADING_RATE_COMBINER combiners[2] = { 
            D3D12Util_TranslateShadingRateCombiner(post_rasterizer_rate), 
            D3D12Util_TranslateShadingRateCombiner(final_rate) };
        ((ID3D12GraphicsCommandList5*)Cmd->pDxCmdList)->RSSetShadingRate(
            D3D12Util_TranslateShadingRate(shading_rate), combiners);
    }
    // TODO: VRS Tier2
    if (Adapter->adapter_detail.support_shading_rate_mask)
    {

    }
#endif
}

#if !defined(XBOX)
    #pragma comment(lib, "dstorage.lib")
#endif

#include "dstorage.h"

struct CGPUDStorageQueueD3D12 : public CGPUDStorageQueue {
    IDStorageQueue* pQueue;
    IDStorageFactory* pFactory;
    SMutex request_mutex;
};

thread_local static IDStorageFactory* factory = nullptr;
ECGPUDStorageAvailability cgpu_query_dstorage_availability_d3d12(CGPUDeviceId device)
{
    if (!factory)
    {
        if (!SUCCEEDED(DStorageGetFactory(IID_PPV_ARGS(&factory))))
        {
            return CGPU_DSTORAGE_AVAILABILITY_NONE;
        }
    }
    return CGPU_DSTORAGE_AVAILABILITY_HARDWARE;
}

CGPUDStorageQueueId cgpu_create_dstorage_queue_d3d12(CGPUDeviceId device, const CGPUDStroageQueueDescriptor* desc)
{
    CGPUDStorageQueueD3D12* Q = SkrNew<CGPUDStorageQueueD3D12>();
    auto Device = (CGPUDevice_D3D12*)device;
    DSTORAGE_QUEUE_DESC queueDesc{};
    queueDesc.Capacity = desc->capacity;
    queueDesc.Priority = (DSTORAGE_PRIORITY)desc->priority;
    queueDesc.SourceType = (DSTORAGE_REQUEST_SOURCE_TYPE)desc->source;
    queueDesc.Name = desc->name;
    queueDesc.Device = Device->pDxDevice;
    if (!factory)
    {
        if (!SUCCEEDED(DStorageGetFactory(IID_PPV_ARGS(&factory))))
        {
            SKR_LOG_ERROR("Failed to get DStorage factory!");
            SkrDelete(Q);
            return nullptr;
        }
    }
    if (!SUCCEEDED(factory->CreateQueue(&queueDesc, IID_PPV_ARGS(&Q->pQueue))))
    {
        SKR_LOG_ERROR("Failed to create DStorage queue!");
        SkrDelete(Q);
        return nullptr;
    }
    Q->pFactory = factory;
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
    if (desc->source_type == CGPU_DSTORAGE_SOURCE_FILE)
    {
        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
        request.Source.File.Source = (IDStorageFile*)desc->source_file.file;
        request.Source.File.Offset = desc->source_file.offset;
        request.Source.File.Size = desc->source_file.size;
    }
    else if (desc->source_type == CGPU_DSTORAGE_SOURCE_MEMORY)
    {
        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_MEMORY;
        request.Source.Memory.Source = desc->source_memory.bytes;
        request.Source.Memory.Size = desc->source_memory.bytes_size;
    }
    request.Destination.Buffer.Resource = pBuffer->pDxResource;
    request.Destination.Buffer.Offset = desc->offset;
    request.Destination.Buffer.Size = desc->size;
    request.UncompressedSize = desc->size;
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
        Q->pQueue->Release();
    }

    SkrDelete(Q);
}