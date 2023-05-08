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

struct SKR_DASCRIPT_API Sequence
{
    bool dispatch(Context* ctx) SKR_NOEXCEPT;

    mutable void* iterator;
};

struct SKR_DASCRIPT_API Context
{
    static Context* Create(const ContextDescriptor& desc) SKR_NOEXCEPT;
    static void Free(Context* ctx) SKR_NOEXCEPT;

    virtual ~Context() SKR_NOEXCEPT;

    virtual FunctionId find_function(const char8_t* name) SKR_NOEXCEPT = 0;

    virtual Register eval(FunctionId func, Register* args = nullptr, Sequence* generated = nullptr) SKR_NOEXCEPT = 0;
    virtual Register eval_with_catch(FunctionId func, Register* args = nullptr, Sequence* generated = nullptr) SKR_NOEXCEPT = 0;

    // template <typename ReturnType, typename ...Args>
    // bool verifyCall(Function* func, const Library* lib); 
};

} // namespace das
} // namespace skr