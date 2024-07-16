#pragma once
#include "SkrProfile/profile.h"
#include "SkrGraphics/dstorage.h"
#include "SkrGraphics/containers.hpp"
#include "SkrOS/shared_library.hpp"
#include "sdk/dstorage.h"

// #define TRACY_PROFILE_DIRECT_STORAGE
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    #include "SkrCore/memory/memory.h"
    #include "SkrBase/atomic/atomic.h"
    #include "SkrOS/thread.h"
#endif

struct SkrWindowsDStorageInstance : public SkrDStorageInstance
{
    static SkrWindowsDStorageInstance* Initialize(const SkrDStorageConfig& cfg);
    static SkrWindowsDStorageInstance* Get();
    ~SkrWindowsDStorageInstance();

    struct ID3D12Device* pDxDevice = nullptr;
    IDStorageFactory* pFactory = nullptr;
    struct DStorageEventPool* pEventPool = nullptr;
    skr::SharedLibrary dstorage_library;
    skr::SharedLibrary dstorage_core;
    bool initialize_failed = false;
    uint64_t sDirectStorageStagingBufferSize = DSTORAGE_STAGING_BUFFER_SIZE_32MB;
    static SkrWindowsDStorageInstance* _this;
};

struct DStorageQueueWindows : public SkrDStorageQueue {
    SkrWindowsDStorageInstance* pInstance;
    struct ID3D12Device* pDxDevice;
    IDStorageQueue* pQueue;
    IDStorageFactory* pFactory;
    uint64_t max_size;
    DSTORAGE_REQUEST_SOURCE_TYPE source_type;
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    SMutex profile_mutex;
    struct ProfileTracer {
        cgpu::String name;
        DStorageQueueWindows* Q;
        ID3D12Fence* fence;
        SThreadDesc desc;
        SThreadHandle thread_handle;
        HANDLE fence_event;
        uint64_t submit_index;
        uint32_t fence_value = 0;
        SAtomicU32 finished;
    };
    cgpu::Vector<ProfileTracer*> profile_tracers;
#endif

    ~DStorageQueueWindows() SKR_NOEXCEPT {
#ifdef TRACY_PROFILE_DIRECT_STORAGE
        for (auto&& tracer : profile_tracers)
        {
            if (!atomic_load_acquire(&tracer->finished))
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

#ifdef TRACY_PROFILE_DIRECT_STORAGE
void skr_dstorage_queue_trace_submit(SkrDStorageQueueId queue);
#endif
