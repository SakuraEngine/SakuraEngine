
#include "stack.hpp"
#include <corecrt_malloc.h>

namespace dual
{
    fixed_stack_t::fixed_stack_t(size_t cap)
        :size(0), capacity(cap)
    {
        buffer = ::malloc( cap);
    }
    fixed_stack_t::~fixed_stack_t()
    {
        ::free(buffer);
    }
    
    void* fixed_stack_t::allocate(size_t inSize)
    {
        auto result = (char*)buffer + size;
        size+=inSize;
        return result;
    }
    void fixed_stack_t::free(size_t inSize)
    {
        size-=inSize;
    }
}