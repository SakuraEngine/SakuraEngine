#pragma once
#include "platform/memory.h"
#include "misc/log.h"
#include "platform/windows/dstorage.h"
#include "platform/dstorage.h"
#include "platform/shared_library.hpp"
#include "platform/atomic.h"
#include "platform/thread.h"

#include "EASTL/vector.h"

#define TRACY_PROFILE_DIRECT_STORAGE
#include "tracy/Tracy.hpp"

struct SkrWindowsDStorageInstance : public SkrDStorageInstance
{
    static SkrWindowsDStorageInstance* Get();
    ~SkrWindowsDStorageInstance();

    IDStorageFactory* pFactory = nullptr;
    skr::SharedLibrary dstorage_library;
    skr::SharedLibrary dstorage_core;
    bool dstorage_dll_dont_exist = false;
    uint64_t sDirectStorageStagingBufferSize = DSTORAGE_STAGING_BUFFER_SIZE_32MB;
};

struct DStorageQueueWindows : public SkrDStorageQueue {
    IDStorageQueue* pQueue;
    IDStorageFactory* pFactory;
    uint64_t max_size;
    DSTORAGE_REQUEST_SOURCE_TYPE source_type;
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    SMutex profile_mutex;
    struct ProfileTracer {
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
