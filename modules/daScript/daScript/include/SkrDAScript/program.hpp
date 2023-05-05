#pragma once
#include "SkrDAScript/env.hpp"

namespace skr {
namespace das {

struct SKR_DASCRIPT_API Program
{
    static void Free(Program* program) SKR_NOEXCEPT;

    virtual ~Program() SKR_NOEXCEPT;

    virtual uint32_t get_ctx_stack_size() SKR_NOEXCEPT = 0;
    virtual bool simulate(Context* ctx, TextPrinter* tout) SKR_NOEXCEPT = 0;
};

} // namespace das
} // namespace skr