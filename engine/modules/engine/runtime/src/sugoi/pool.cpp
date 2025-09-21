#include "SkrProfile/profile.h"
#include "SkrRuntime/sugoi/sugoi_config.h"
#include "SkrContainers/vector.hpp"

#include "./pool.hpp"
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
        return sugoi_malloc(blockSize);
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
    buffer = (char*)sugoi_malloc(blockSize * blockCount);
    skr::Vector<size_t> indicies;
    indicies.resize_default(blockCount);
    std::iota(indicies.begin(), indicies.end(), 0);
    blocks.try_enqueue_bulk(indicies.data(), blockCount);
}

fixed_pool_t::~fixed_pool_t()
{
    sugoi_free(buffer);
}

void* fixed_pool_t::allocate()
{
    size_t block;
    if (blocks.try_dequeue(block))
    {
        // 验证索引有效性
        if (block < blockCount)
            return (char*)buffer + block * blockSize;
        
        // 如果索引无效，说明队列数据损坏，记录错误并返回nullptr
        // 在调试模式下，这可能表明队列管理存在严重问题
        return nullptr;
    }
    return nullptr;
}

void fixed_pool_t::free(void* block)
{
    // 检查指针是否在有效范围内
    if (block < buffer || block >= (char*)buffer + blockSize * blockCount)
        return;
    
    // 检查指针对齐 - 必须是块的起始地址
    size_t offset = (char*)block - (char*)buffer;
    if (offset % blockSize != 0)
        return;
    
    size_t block_index = offset / blockSize;
    
    // 双重检查：索引必须在有效范围内
    if (block_index >= blockCount)
        return;
    
    if (blocks.try_enqueue(block_index))
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