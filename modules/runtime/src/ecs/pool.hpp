#pragma once
#include "containers/concurrent_queue.h"

namespace dual
{
    
struct ECSPoolConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
{
    static constexpr const char* kECSPoolQueueName = "ECSPool";
    static const bool RECYCLE_ALLOCATED_BLOCKS = true;
    static inline void* malloc(size_t size) { return sakura_mallocN(size, kECSPoolQueueName); }
    static inline void free(void* ptr) { return sakura_freeN(ptr, kECSPoolQueueName); }
};
struct pool_t {
    size_t blockSize;
    skr::ConcurrentQueue<void*, ECSPoolConcurrentQueueTraits> blocks;
    pool_t(size_t blockSize, size_t blockCount);
    ~pool_t();
    void* allocate();
    void free(void* block);
};

pool_t& get_default_pool();
pool_t& get_default_pool_small();
pool_t& get_default_pool_large();

struct fixed_pool_t {
    char* buffer;
    size_t blockSize;
    size_t blockCount;
    skr::ConcurrentQueue<size_t, ECSPoolConcurrentQueueTraits> blocks;
    fixed_pool_t(size_t blockSize, size_t blockCount);
    ~fixed_pool_t();
    void* allocate();
    void free(void* block);
    void reset();
};
} // namespace dual