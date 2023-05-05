#include "SkrDAScript/ctx.hpp"
#include "utils/log.h"
#include "daScript/simulate/simulate.h"

namespace skr {
namespace das {

struct ScriptContext final : public ::das::Context
{
    using Super = ::das::Context;
    ScriptContext(uint32_t stackSize) : Super(stackSize) {}
    ScriptContext(Super& ctx, uint32_t category) : Super(ctx, category) {}

    void to_out(const char* message) { SKR_LOG_INFO(message); }
    void to_err(const char* message) { SKR_LOG_ERROR(message); }
};

Context::~Context() SKR_NOEXCEPT
{

}

struct ContextImpl : public Context
{
    ~ContextImpl() SKR_NOEXCEPT {}
    class ::das::Context* get_context() SKR_NOEXCEPT override { return &ctx; }

    ScriptContext ctx;
};

} // namespace das
} // namespace skr