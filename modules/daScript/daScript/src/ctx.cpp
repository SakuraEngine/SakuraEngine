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

Register ContextImpl::eval(Function func, Register* args) SKR_NOEXCEPT
{
    static_assert(sizeof(skr_float4_t) == sizeof(vec4f), "size not match");
    static_assert(alignof(skr_float4_t) == alignof(vec4f), "align not match");

    auto res = ctx.eval((const SimFunction*)func.ptr, (vec4f*)args, nullptr);
    auto result = *(Register*)&res;
    return result;
}

Register ContextImpl::eval_with_catch(Function func, Register* args) SKR_NOEXCEPT
{
    static_assert(sizeof(skr_float4_t) == sizeof(vec4f), "size not match");
    static_assert(alignof(skr_float4_t) == alignof(vec4f), "align not match");

    auto res = ctx.evalWithCatch((SimFunction*)func.ptr, (vec4f*)args, nullptr);
    return *(Register*)&res;
}

} // namespace das
} // namespace skr