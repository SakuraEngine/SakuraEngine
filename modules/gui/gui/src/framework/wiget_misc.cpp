#include "SkrGui/framework/widget_misc.hpp"

namespace skr::gui
{
FixedStack::FixedStack(size_t cap)
    : size(0)
    , capacity(cap)
{
    buffer = ::malloc(cap);
}
FixedStack::~FixedStack()
{
    ::free(buffer);
}

void* FixedStack::allocate(size_t inSize)
{
    auto result = (char*)buffer + size;
    size += inSize;
    return result;
}
void FixedStack::free(size_t inSize)
{
    size -= inSize;
}

FixedStack& get_default_fixed_stack()
{
    thread_local static FixedStack stack(4096 * 8);
    return stack;
}
} // namespace skr::gui