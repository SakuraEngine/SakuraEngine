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
    return program->simulate(Ctx->ctx, TOut->printer);
}

} // namespace das
} // namespace skr