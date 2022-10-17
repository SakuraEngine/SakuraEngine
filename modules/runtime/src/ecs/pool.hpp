#pragma once
#include "ftl/coqueue.h"

namespace dual
{
struct pool_t {
    size_t blockSize;
    moodycamel::ConcurrentQueue<void*> blocks;
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
    moodycamel::ConcurrentQueue<size_t> blocks;
    fixed_pool_t(size_t blockSize, size_t blockCount);
    ~fixed_pool_t();
    void* allocate();
    void free(void* block);
    void reset();
};
} // namespace dual