#include "arena.hpp"
#include "pool.hpp"
#include "utils.hpp"
#include <assert.h>

namespace dual
{
fixed_arena_t::fixed_arena_t(size_t capacity)
    : buffer(::dual_malloc(capacity))
    , size()
    , capacity(capacity)
{
}

fixed_arena_t::~fixed_arena_t()
{
    if (buffer)
        ::dual_free(buffer);
}

void fixed_arena_t::forget()
{
    buffer = nullptr;
}

void* fixed_arena_t::allocate(size_t s, size_t a)
{
    size_t offset = size.fetch_add(s + a - 1);
    offset = ((offset + a - 1) / a) * a;
    assert(offset < capacity);
    return (char*)buffer + offset;
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
} // namespace dual