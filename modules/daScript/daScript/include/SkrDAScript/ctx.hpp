#pragma once
#include "SkrDAScript/function.hpp"
#include "SkrDAScript/file.hpp"

namespace das { class Context; }

namespace skr {
namespace das {

struct ContextDescriptor
{
    uint32_t stack_size = 0;
};

struct SKR_DASCRIPT_API Context
{
    static Context* Create(const ContextDescriptor& desc) SKR_NOEXCEPT;
    static void Free(Context* ctx) SKR_NOEXCEPT;

    virtual ~Context() SKR_NOEXCEPT;

    virtual Function find_function(const char8_t* name) SKR_NOEXCEPT = 0;
    virtual Register eval(Function func, Register* args = nullptr) SKR_NOEXCEPT = 0;
    virtual Register eval_with_catch(Function func, Register* args = nullptr) SKR_NOEXCEPT = 0;
    // virtual class ::das::Context* get_context() SKR_NOEXCEPT = 0;
};

} // namespace das
} // namespace skr