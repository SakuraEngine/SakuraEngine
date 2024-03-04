#include "SkrProfile/profile.h"
#include "SkrRT/ecs/sugoi_config.h"
#include "SkrContainers/vector.hpp"
#include "SkrRT/ecs/detail/pool.hpp"
#include <numeric>

const char* kDualMemoryName = "sugoi";
namespace sugoi
{
pool_t::pool_t(size_t blockSize, size_t blockCount)
    : blockSize(blockSize)
    , blocks(blockCount)
{
}

pool_t::~pool_t()
{
    void* block;
    while (blocks.try_dequeue(block))
        sugoi_free(block);
}

void* pool_t::allocate()
{
    void* block;
    if (blocks.try_dequeue(block))
        return block;
    {
        SkrZoneScopedN("DualPoolAllocation");
        return sugoi_calloc(1, blockSize);
    }
}

void pool_t::free(void* block)
{
    if (blocks.try_enqueue(block))
        return;
    sugoi_free(block);
}

fixed_pool_t::fixed_pool_t(size_t blockSize, size_t blockCount)
    : blockSize(blockSize)
    , blockCount(blockCount)
    , blocks(blockCount)
{
    buffer = new char[blockSize * blockCount];
    skr::Vector<size_t> indicies;
    indicies.resize_default(blockCount);
    std::iota(indicies.begin(), indicies.end(), 0);
    for (size_t i = 0; i < blockCount; ++i)
        blocks.try_enqueue_bulk(indicies.data(), blockCount);
}

fixed_pool_t::~fixed_pool_t()
{
    delete buffer;
}

void* fixed_pool_t::allocate()
{
    size_t block;
    if (blocks.try_dequeue(block))
        return buffer + block * blockSize;
    return nullptr;
}

void fixed_pool_t::free(void* block)
{
    if (block < buffer || block > buffer + blockSize * blockCount)
        return;
    if (blocks.try_enqueue(((char*)block - buffer) / blockSize))
        return;
}

void fixed_pool_t::reset()
{
    skr::ConcurrentQueue<size_t, ECSPoolConcurrentQueueTraits> temp(blockCount);
    blocks.swap(temp);
    skr::Vector<size_t> indicies;
    indicies.resize_default(blockCount);
    std::iota(indicies.begin(), indicies.end(), 0);
    for (size_t i = 0; i < blockCount; ++i)
        blocks.try_enqueue_bulk(indicies.data(), blockCount);
}
} // namespace sugoi