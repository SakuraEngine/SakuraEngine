#include "types.hpp"

namespace skr {
namespace das {

Program::~Program() SKR_NOEXCEPT
{

}

void Program::Free(Program* program) SKR_NOEXCEPT
{
    SkrDelete(program);
}

uint32_t ProgramImpl::get_ctx_stack_size() SKR_NOEXCEPT
{
    return (uint32_t)program->getContextStackSize();
}

bool ProgramImpl::simulate(Context* ctx, TextPrinter* tout) SKR_NOEXCEPT
{
    auto Ctx = static_cast<ContextImpl*>(ctx);
    auto TOut = static_cast<TextPrinterImpl*>(tout);
    if (!program->simulate(Ctx->ctx, TOut->printer))
    {
        Ctx->ctx.to_err("Failed to simulate\n");        
        for ( auto & err : program->errors ) {
            Ctx->ctx.to_err(
                reportError(err.at, err.what, err.extra, err.fixme, err.cerr).c_str()
            );
        }
        return false;
    }
    return true;
}

} // namespace das
} // namespace skr