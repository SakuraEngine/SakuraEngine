#include "SkrRT/ecs/sugoi_config.h"
#include "./stack.hpp"

namespace sugoi
{
fixed_stack_t::fixed_stack_t(size_t cap)
    : size(0)
    , capacity(cap)
{
    buffer = ::sugoi_malloc(cap);
}

fixed_stack_t::~fixed_stack_t()
{
    ::sugoi_free(buffer);
}

void* fixed_stack_t::allocate(size_t inSize)
{
    auto result = (char*)buffer + size;
    size += inSize;
    return result;
}

void fixed_stack_t::free(size_t inSize)
{
    size -= inSize;
}
} // namespace sugoi