#include "SkrRT/platform/win/dstorage_windows.h"
#include "SkrRT/platform/memory.h"
#include "SkrRT/platform/thread.h"
#include "SkrRT/misc/log.h"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/async/thread_job.hpp"

#include "platform/windows/windows_dstorage.hpp"
#include <EASTL/string.h>
#include <EASTL/vector_map.h>

static void CALLBACK __decompressThreadPoolTask_DirectStorage(
    TP_CALLBACK_INSTANCE*,
    void*,
    TP_WAIT* wait,
    TP_WAIT_RESULT);

static void CALLBACK __decompressThreadTask_DirectStorage(void*);

struct skr_win_dstorage_decompress_service_t
{
    const bool use_thread_pool = true;
    skr_win_dstorage_decompress_service_t(IDStorageCustomDecompressionQueue* queue, const skr_win_dstorage_decompress_desc_t* desc)
        : decompress_queue(queue), job_queue(desc->job_queue)
    {
        skr_atomicu32_store_release(&thread_running, 1);
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
    ~skr_win_dstorage_decompress_service_t() SKR_NOEXCEPT
    {
        skr_atomicu32_store_release(&thread_running, 0);
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
    eastl::vector_map<SkrDStorageCompression, DecompressionResolver> resolvers;
    // threadpool items
    TP_WAIT* thread_pool_wait = nullptr;
    // thread items
    SThreadDesc thread_desc;
    SThreadHandle thread_handle = nullptr;
    SAtomicU32 thread_running;

    skr::JobQueue* job_queue = nullptr;
    skr::vector<skr::IFuture<bool>*> decompress_futures;
};

using DSDecompressFutureLauncher = skr::FutureLauncher<bool>;

static void __decompressTask_DirectStorage(skr_win_dstorage_decompress_service_id service)
{
    auto running = skr_atomicu32_load_acquire(&service->thread_running);
    while (running)
    {
        DSTORAGE_CUSTOM_DECOMPRESSION_REQUEST requests[64];
        uint32_t numRequests = 0;

        // Pull off a batch of requests to process in this loop.
        service->decompress_queue->GetRequests(_countof(requests), requests, &numRequests);

        if (numRequests == 0)
            break;

        {
            SkrZoneScopedN("DirectStorageDecompress");
            for (uint32_t i = 0; i < numRequests; ++i)
            {
                auto& future = service->decompress_futures.emplace_back();
                future = DSDecompressFutureLauncher(service->job_queue).async(
                [service, request = requests[i]](){
                    auto resolver = service->resolvers.find(request.CompressionFormat);
                    if (resolver != service->resolvers.end())
                    {
                        auto skrRequest = make_zeroed<skr_win_dstorage_decompress_request_t>();
                        skrRequest.id = request.Id;
                        skrRequest.compression = request.CompressionFormat;
                        skrRequest.flags = request.Flags;
                        skrRequest.src_size = request.SrcSize;
                        skrRequest.src_buffer = request.SrcBuffer;
                        skrRequest.dst_size = request.DstSize;
                        skrRequest.dst_buffer = request.DstBuffer;
                        auto result = resolver->second.callback(&skrRequest, resolver->second.user_data);
                        DSTORAGE_CUSTOM_DECOMPRESSION_RESULT failResult = {};
                        failResult.Result = result;
                        failResult.Id = request.Id;
                        service->decompress_queue->SetRequestResults(1, &failResult);
                    }
                    else
                    {
                        SKR_LOG_WARN(u8"Unable to find decompression resolver for format %d\n", request.CompressionFormat);
                        DSTORAGE_CUSTOM_DECOMPRESSION_RESULT failResult = {};
                        failResult.Result = S_FALSE;
                        failResult.Id = request.Id;
                        service->decompress_queue->SetRequestResults(1, &failResult);
                    }
                    return true;
                });
            }
        }
        {
            SkrZoneScopedN("CleanupFutures");
            auto& arr = service->decompress_futures;
            for (auto& future : arr)
            {
                auto status = future->wait_for(0);
                if (status == skr::FutureStatus::Ready)
                {
                    SkrDelete(future);
                    future = nullptr;
                }
            }
            auto it = eastl::remove_if(arr.begin(), arr.end(), 
                [](skr::IFuture<bool>* future) {
                    return (future == nullptr);
                });
            arr.erase(it, arr.end());
        }
    }
}

static void CALLBACK __decompressThreadPoolTask_DirectStorage(
    TP_CALLBACK_INSTANCE*,
    void* data,
    TP_WAIT* wait,
    TP_WAIT_RESULT)
{
#ifdef SKR_PROFILE_ENABLE
    auto thread_id = skr_current_thread_id();
    const auto indexed_name = skr::format(u8"DirectStorageDecompressThread(Pooled)-{}", thread_id);
    tracy::SetThreadName(indexed_name.c_str());
#endif

    auto service = (skr_win_dstorage_decompress_service_id)data;
    auto oldPriority = GetThreadPriority(GetCurrentThread());
    {
        SkrZoneScopedN("DirectStorageDecompress");
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

        __decompressTask_DirectStorage(service);
    }
    

    // Restore the original thread's priority back to its original setting.
    SetThreadPriority(GetCurrentThread(), oldPriority);
    // Re-register the custom decompression queue event with this callback
    // to be called when the next decompression requests become available for processing.
    SetThreadpoolWait(wait, service->event_handle, nullptr);
}

static void CALLBACK __decompressThreadTask_DirectStorage(void* data)
{
#ifdef SKR_PROFILE_ENABLE
    tracy::SetThreadName("DirectStorageDecompressThread");
#endif
    auto service = (skr_win_dstorage_decompress_service_id)data;
    while (skr_atomicu32_load_acquire(&service->thread_running))
    {
        {
            SkrZoneScopedNC("DirectStorageDecompressWait", tracy::Color::Gray43);
            WaitForSingleObject(service->event_handle, INFINITE);
        }
        __decompressTask_DirectStorage(service);
    } 
}

void skr_win_dstorage_set_staging_buffer_size(uint64_t size)
{
    auto _this = (SkrWindowsDStorageInstance*)skr_get_dstorage_instnace();
    if (!_this) return;
    if (!_this->pFactory) return;
    _this->pFactory->SetStagingBufferSize((uint32_t)size);
    _this->sDirectStorageStagingBufferSize = size;
}

skr_win_dstorage_decompress_service_id skr_win_dstorage_create_decompress_service(const skr_win_dstorage_decompress_desc_t* desc)
{
    auto _this = (SkrWindowsDStorageInstance*)skr_get_dstorage_instnace();
    if (!_this) return nullptr;
    if (!_this->pFactory) return nullptr;

    IDStorageFactory* pFactory = _this->pFactory;
    if (!pFactory) return nullptr;
    IDStorageCustomDecompressionQueue* pCompressionQueue = nullptr;
    if (!SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&pCompressionQueue))))
    {
        return nullptr;
    }
    auto service = SkrNew<skr_win_dstorage_decompress_service_t>(pCompressionQueue, desc);
    SKR_LOG_TRACE(u8"Created decompress service");
    return service;
}

bool skr_win_dstorage_decompress_service_register_callback(skr_win_dstorage_decompress_service_id service, 
    SkrDStorageCompression compression, skr_win_dstorage_decompress_callback_t callback, void* user_data)
{
    const auto registered = (service->resolvers.find(compression) != service->resolvers.end());
    SKR_ASSERT(!registered && "Callback already registered for this compression");
    if (registered) return false;
    SKR_ASSERT(callback && "Callback must be valid");
    service->resolvers[compression] = { callback, user_data };
    return true;
}

void skr_win_dstorage_free_decompress_service(skr_win_dstorage_decompress_service_id service)
{
    SKR_ASSERT(service && "Invalid service");
    SkrDelete(service);
    SKR_LOG_TRACE(u8"Deleted decompress service");
}