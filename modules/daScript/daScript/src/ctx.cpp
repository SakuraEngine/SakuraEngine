#include "types.hpp"

#include "daScript/misc/platform.h"
#include "daScript/simulate/simulate.h"

namespace skr {
namespace das {
using namespace ::das;

Context::~Context() SKR_NOEXCEPT
{

}

Context* Context::Create(const ContextDescriptor &desc) SKR_NOEXCEPT
{
    return SkrNew<ContextImpl>(desc.stack_size);
}

void Context::Free(Context *ctx) SKR_NOEXCEPT
{
    SkrDelete(ctx);
}

Function ContextImpl::find_function(const char8_t* name) SKR_NOEXCEPT
{
    void* ptr = ctx.findFunction((const char*)name);
    return Function(ptr);
}

void ContextImpl::eval(Function func) SKR_NOEXCEPT
{
    ctx.eval((const SimFunction*)func.ptr);
}

} // namespace das
} // namespace skr