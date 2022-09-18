#pragma once
#include <atomic>

namespace dual
{
    struct fixed_arena_t
    {
        void* buffer;
        std::atomic<size_t> size;
        size_t capacity;
        fixed_arena_t(size_t capacity);
        ~fixed_arena_t();
        void forget();
        void* allocate(size_t size, size_t align);
        template<class T>
        T* allocate() { return (T*)allocate(sizeof(T), alignof(T)); }
        template<class T>
        T* allocate(size_t size) { return (T*)allocate(sizeof(T)*size, alignof(T)); }
    };

    struct pool_t;
    struct block_arena_t
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
        block_arena_t(pool_t& pool);
        ~block_arena_t();
        void reset();
        template<class T>
        T* allocate()
        {
            return (T*)allocate(sizeof(T), alignof(T));
        }
        template<class T>
        T* allocate(size_t size)
        {
            return (T*)allocate(sizeof(T)*size, alignof(T));
        }
        void* allocate(size_t size, size_t align);
    };
}