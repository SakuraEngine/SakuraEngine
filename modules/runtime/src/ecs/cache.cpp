#include "cache.hpp"

dual::cache_base_t::cache_base_t(pool_t& pool)
    : pool(pool), first(nullptr), last(nullptr), curr(0), count(0) {}

dual::cache_base_t::~cache_base_t()
{
    auto iter = first;
    while(iter != nullptr)
    {
        auto next = iter->next;
        pool.free(iter);
        iter = next;
    }
}