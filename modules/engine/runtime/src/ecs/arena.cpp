#include "SkrProfile/profile.h"
#include "SkrRT/ecs/sugoi_config.h"

#include "./arena.hpp"
#include "./pool.hpp"

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
    //TODO: waste memory
    size_t offset = size.fetch_add(s + a - 1);
    offset = ((offset + a - 1) / a) * a;
    SKR_ASSERT(offset < capacity);
    return (char*)buffer + offset;
}

void* struct_arena_base_t::allocate(size_t s, size_t a)
{
    size = ((size + a - 1) / a) * a;
    SKR_ASSERT(size + s <= capacity);
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
    if (s > pool.blockSize)
        return nullptr;
    curr = ((curr + a - 1) / a) * a;
    if (first == nullptr)
    {
        first = last = (block_t*)pool.allocate();
        first->next = nullptr;
    }
    if (curr + s > pool.blockSize)
    {
        last->next = (block_t*)pool.allocate();
        last = last->next;
        last->next = nullptr;
        curr = 0;
    }
    void* result = (char*)last->data() + curr;
    curr += s;
    return result;
}
} // namespace sugoi