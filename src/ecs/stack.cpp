
#include "stack.hpp"
#include "utils.hpp"

namespace dual
{
fixed_stack_t::fixed_stack_t(size_t cap)
    : size(0)
    , capacity(cap)
{
    buffer = dual_malloc(cap);
}
fixed_stack_t::~fixed_stack_t()
{
    dual_free(buffer);
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
} // namespace dual