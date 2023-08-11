#pragma once
#include "SkrRT/platform/memory.h"
#include "SkrRT/misc/log.h"
#include "SkrRT/platform/dstorage.h"
#include "SkrRT/platform/shared_library.hpp"
#include "SkrRT/platform/atomic.h"
#include "SkrRT/platform/thread.h"
#include "platform/windows/dstorage.h"

#include "EASTL/vector.h"

// #define TRACY_PROFILE_DIRECT_STORAGE
#include "SkrProfile/profile.h"

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
        skr::string name;
        DStorageQueueWindows* Q;
        ID3D12Fence* fence;
        SThreadDesc desc;
        SThreadHandle thread_handle;
        HANDLE fence_event;
        uint64_t submit_index;
        uint32_t fence_value = 0;
        SAtomicU32 finished;
    };
    eastl::vector<ProfileTracer*> profile_tracers;
#endif

    ~DStorageQueueWindows() SKR_NOEXCEPT {
#ifdef TRACY_PROFILE_DIRECT_STORAGE
        for (auto&& tracer : profile_tracers)
        {
            if (!skr_atomicu32_load_acquire(&tracer->finished))
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
