#pragma once
#include "misc/smart_pool.hpp"

namespace skr {
namespace io {

extern const char* kIOPoolObjectsMemoryName; 
extern const char* kIOConcurrentQueueName;
struct IOConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
{
    static const bool RECYCLE_ALLOCATED_BLOCKS = true;
    static const size_t BLOCK_SIZE = 32;
    static inline void* malloc(size_t size) { return sakura_mallocN(size, kIOConcurrentQueueName); }
    static inline void free(void* ptr) { return sakura_freeN(ptr, kIOConcurrentQueueName); }
};
template<typename T>
using IOConcurrentQueue = moodycamel::ConcurrentQueue<T, IOConcurrentQueueTraits>;  

} // namespace io
} // namespace skr

#define IO_RC_OBJECT_BODY SKR_RC_OBJECT_BODY