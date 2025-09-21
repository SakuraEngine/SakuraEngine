#include "SkrProfile/profile.h"
#include "SkrRuntime/sugoi/sugoi_config.h"

#include "./arena.hpp"
#include "./pool.hpp"

#include <cstdint>

namespace sugoi
{
fixed_arena_t::fixed_arena_t(size_t capacity)
    : size()
    , capacity(capacity)
{
    SkrZoneScopedN("DualFixedArenaAllocation");
    
    buffer = sugoi_calloc(1, capacity);
}

fixed_arena_t::~fixed_arena_t()
{
    if (buffer)
        sugoi_free(buffer);
}

void fixed_arena_t::forget()
{
    buffer = nullptr;
}

void* fixed_arena_t::allocate(size_t s, size_t a)
{
    // 检查对齐参数是否有效
    if (a == 0 || (a & (a - 1)) != 0)
        return nullptr;
    
    // 检查分配大小是否合理
    if (s == 0 || s > capacity)
        return nullptr;
    
    // 安全的对齐计算，防止溢出
    size_t current_size = size.load();
    size_t aligned_offset;
    
    do {
        // 检查溢出
        if (current_size > SIZE_MAX - (a - 1))
            return nullptr;
        
        aligned_offset = ((current_size + a - 1) / a) * a;
        
        // 检查是否超出容量
        if (aligned_offset > capacity - s)
            return nullptr;
        
        // 尝试原子性地更新大小
    } while (!size.compare_exchange_weak(current_size, aligned_offset + s));
    
    return (char*)buffer + aligned_offset;
}

void* struct_arena_base_t::allocate(size_t s, size_t a)
{
    // 检查对齐参数是否有效
    if (a == 0 || (a & (a - 1)) != 0)
        return nullptr;
    
    // 检查分配大小是否合理
    if (s == 0)
        return nullptr;
    
    // 安全的对齐计算，防止溢出
    if (size > SIZE_MAX - (a - 1))
        return nullptr;
    
    size = ((size + a - 1) / a) * a;
    
    // 检查是否超出容量
    if (size > capacity - s)
        return nullptr;
    
    size += s;
    return (char*)buffer + size - s;
}

void struct_arena_base_t::initialize(size_t a)
{
    SkrZoneScopedN("DualArenaAllocation");

    buffer = sugoi_calloc_aligned(1, capacity, a);
}

void struct_arena_base_t::record(size_t s, size_t a)
{
    capacity = ((capacity + a - 1) / a) * a;
    capacity += s;
}

block_arena_t::block_arena_t(pool_t& pool)
    : pool(pool)
    , first(nullptr)
    , last(nullptr)
    , curr(0)
{
}

block_arena_t::~block_arena_t()
{
    reset();
}

void block_arena_t::reset()
{
    auto iter = first;
    while (iter != nullptr)
    {
        auto next = iter->next;
        pool.free(iter);
        iter = next;
    }
    curr = 0;
    last = first = nullptr;
}

void* block_arena_t::allocate(size_t s, size_t a)
{
    // 检查分配大小是否超出块大小（需要减去块头大小）
    constexpr size_t block_header_size = sizeof(block_t);
    if (pool.blockSize < block_header_size || s > pool.blockSize - block_header_size)
        return nullptr;
    
    // 检查对齐参数是否有效
    if (a == 0 || (a & (a - 1)) != 0)
        return nullptr;
    
    // 安全的对齐计算，防止溢出
    size_t aligned_curr;
    if (curr > SIZE_MAX - (a - 1))
        return nullptr; // 溢出检查
    
    aligned_curr = ((curr + a - 1) / a) * a;
    
    if (first == nullptr)
    {
        first = last = (block_t*)pool.allocate();
        if (first == nullptr)
            return nullptr; // 处理分配失败
        first->next = nullptr;
        aligned_curr = 0; // 新块从0开始
    }
    
    // 检查当前块是否有足够空间（考虑块头大小）
    if (aligned_curr + s > pool.blockSize - block_header_size)
    {
        // 分配新块
        block_t* new_block = (block_t*)pool.allocate();
        if (new_block == nullptr)
            return nullptr; // 处理分配失败
            
        last->next = new_block;
        last = new_block;
        last->next = nullptr;
        aligned_curr = 0; // 新块从0开始
    }
    
    curr = aligned_curr;
    void* result = (char*)last->data() + curr;
    curr += s;
    return result;
}
} // namespace sugoi