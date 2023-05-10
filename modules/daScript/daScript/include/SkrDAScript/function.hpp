#pragma once
#include "SkrDAScript/env.hpp"

namespace skr {
namespace das {

struct ContextImpl;
struct SKR_DASCRIPT_API SimFunctionId
{
    friend struct skr::das::Context;
    friend struct skr::das::ContextImpl;
    ~SimFunctionId() SKR_NOEXCEPT;
    operator bool() const { return ptr; }
    const char8_t* get_name() const;
    const char8_t* get_mangled_name() const;
    const uint64_t get_mangled_name_hash() const;
    const uint64_t get_stack_size() const;

protected:
    SimFunctionId(void* ptr) SKR_NOEXCEPT;
    void* ptr = nullptr;
};

/*
struct SKR_DASCRIPT_API Function
{
    virtual ~Function() SKR_NOEXCEPT;
    virtual skr::text::text get_mangled_name() const SKR_NOEXCEPT = 0;
    virtual skr::text::text get_aot_name() const SKR_NOEXCEPT = 0;
    virtual skr::text::text describe_name() const SKR_NOEXCEPT = 0;
    virtual skr::text::text describe() const SKR_NOEXCEPT = 0;
    virtual bool is_generic() const SKR_NOEXCEPT = 0;
    virtual Function* get_origin() const SKR_NOEXCEPT = 0;
};
*/
} // namespace das
} // namespace skr