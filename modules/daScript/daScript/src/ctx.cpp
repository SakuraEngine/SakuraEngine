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

bool Sequence::dispatch(Context* ctx) SKR_NOEXCEPT
{
    auto Ctx = static_cast<ContextImpl*>(ctx);
    bool dummy = false;
    auto& seq = *(::das::Sequence*)this;
    return ::das::builtin_iterator_iterate(seq, &dummy, &Ctx->ctx);
}

void ScriptContext::to_out(const char* message) { log_log(SKR_LOG_LEVEL_INFO, this->name.c_str(), 0, message); }
void ScriptContext::to_err(const char* message) { log_log(SKR_LOG_LEVEL_ERROR, this->name.c_str(), 0, message); }

FunctionId ContextImpl::find_function(const char8_t* name) SKR_NOEXCEPT
{
    ::das::SimFunction* ptr = ctx.findFunction((const char*)name);
    return FunctionId(ptr);
}

Register ContextImpl::eval(FunctionId func, Register* args, Sequence* generated) SKR_NOEXCEPT
{
    static_assert(sizeof(skr_float4_t) == sizeof(vec4f), "size not match");
    static_assert(alignof(skr_float4_t) == alignof(vec4f), "align not match");

    static_assert(sizeof(::das::Sequence) == sizeof(Sequence), "size not match");
    static_assert(alignof(::das::Sequence) == alignof(Sequence), "align not match");

    auto res = ctx.eval((const SimFunction*)func.ptr, (vec4f*)args, generated);
    auto result = *(Register*)&res;
    return result;
}

Register ContextImpl::eval_with_catch(FunctionId func, Register* args, Sequence* generated) SKR_NOEXCEPT
{
    static_assert(sizeof(skr_float4_t) == sizeof(vec4f), "size not match");
    static_assert(alignof(skr_float4_t) == alignof(vec4f), "align not match");

    static_assert(sizeof(::das::Sequence) == sizeof(Sequence), "size not match");
    static_assert(alignof(::das::Sequence) == alignof(Sequence), "align not match");

    auto res = ctx.evalWithCatch((SimFunction*)func.ptr, (vec4f*)args, generated);
    return *(Register*)&res;
}

} // namespace das
} // namespace skr