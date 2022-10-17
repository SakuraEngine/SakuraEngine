#pragma once
#include "pool.hpp"
#include <iterator>

namespace dual
{
    struct cache_base_t
    { 
        pool_t& pool;
        struct block_t
        {
            block_t* next;
            void* data() { return (this+1); }
        };
        block_t* first;
        block_t* last;
        size_t curr;
        size_t count;
        cache_base_t(pool_t& pool);
        ~cache_base_t();
    };

    template<class T>
    struct cache_t : cache_base_t
    {
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = size_t;
        cache_t(pool_t& pool)
            :cache_base_t(pool) {}

        template<class... Ts>
        void push(Ts&&... data)
        {
            new (allocate()) T{std::forward<Ts>(data)...};
        }

        void* allocate()
        {
            static constexpr size_t size = sizeof(T);
            if (first == nullptr)
            {
                first = last = (block_t*)pool.allocate();
                first->next = nullptr;
            }
            else if(curr + size > pool.blockSize)
            {
                last->next = (block_t*)pool.allocate();
                last = last->next;
                last->next = nullptr;
                curr = 0;
            }
            void* result = (char*)last->data()+curr;
            curr += size;
            ++count;
            return result;
        }

        void to_array(T* ptr)
        {
            auto iter = first;
            size_t elemPerBlock = (pool.blockSize / sizeof(T));
            size_t remain = count;
            while(remain != 0)
            {
                auto toCopy = std::min(remain, elemPerBlock);
                std::memcpy(ptr, iter->data(), toCopy * sizeof(T));
                ptr += toCopy;
                remain -= toCopy;
                iter = iter->next;
            }
        }
    };
}