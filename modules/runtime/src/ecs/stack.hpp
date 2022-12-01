#pragma once

namespace dual
{
    struct fixed_stack_t
    {
        void* buffer;
        size_t size;
        size_t capacity;
        fixed_stack_t(size_t capacity);
        ~fixed_stack_t();
        void* allocate(size_t size);
        void free(size_t size);
        template<class T>
        T* allocate() { return (T*)allocate(sizeof(T)); }
        template<class T>
        T* allocate(size_t size) { return (T*)allocate(sizeof(T)*size); }
        template<class T>
        void free() { free(sizeof(T)); }
        template<class T>
        void free(size_t size) { free(sizeof(T)*size); }
    };

    struct fixed_stack_scope_t
    {
        size_t top;
        fixed_stack_t& stack;
        fixed_stack_scope_t(fixed_stack_t& stack)
            :top(stack.size), stack(stack) {}
        ~fixed_stack_scope_t()
        {
            stack.size = top;
        }
    };
}