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

    struct struct_arena_base_t
    {
        void* buffer = nullptr;
        size_t size = 0;
        size_t capacity = 0;
        void* allocate(size_t size, size_t align);
        void initialize(size_t align);
        void record(size_t size, size_t align);
    };

    template<class S>
    struct struct_arena_t : private struct_arena_base_t
    {
        using base = struct_arena_base_t;
        struct_arena_t() { base::record(sizeof(S), alignof(S)); }
        template<class T>
        void record(T S::*, size_t size) 
        { 
            using E = std::remove_pointer_t<T>;
            return base::record(sizeof(E) * size, alignof(E)); 
        }
        S* end() { base::initialize(alignof(S)); return new (base::allocate(sizeof(S), alignof(S))) S(); }
        template<class T>
        T get(T S::*, size_t size) 
        { 
            using E = std::remove_pointer_t<T>;
            return (E*)base::allocate(sizeof(E) * size, alignof(E)); 
        }
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