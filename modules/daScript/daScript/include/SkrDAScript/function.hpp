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

struct SKR_DASCRIPT_API FunctionId
{
    friend struct skr::das::Context;
    friend struct skr::das::ContextImpl;
    ~FunctionId() SKR_NOEXCEPT;
    skr::text::text get_mangled_name() const SKR_NOEXCEPT;
    skr::text::text get_aot_name() const SKR_NOEXCEPT;
    skr::text::text describe_name() const SKR_NOEXCEPT;
    skr::text::text describe() const SKR_NOEXCEPT;
    bool is_generic() const SKR_NOEXCEPT;
    Function* get_origin() const SKR_NOEXCEPT;

protected:
    FunctionId(void* ptr) SKR_NOEXCEPT;
    void* ptr = nullptr;
};

struct BuiltinFunctionDescriptor
{
    bool call_based = false;
    bool interop_fn = false;
};

struct SKR_DASCRIPT_API BuiltinFunction
{
    BuiltinFunction* Create() SKR_NOEXCEPT;
    void Free(BuiltinFunction* fn) SKR_NOEXCEPT;

    virtual ~BuiltinFunction() SKR_NOEXCEPT;
    virtual skr::text::text get_mangled_name() const SKR_NOEXCEPT = 0;
    virtual skr::text::text get_aot_name() const SKR_NOEXCEPT = 0;
    virtual skr::text::text describe_name() const SKR_NOEXCEPT = 0;
    virtual skr::text::text describe() const SKR_NOEXCEPT = 0;
    virtual bool is_generic() const SKR_NOEXCEPT = 0;
    virtual Function* get_origin() const SKR_NOEXCEPT = 0;
    BuiltinFunction* arg(const char8_t* argName) SKR_NOEXCEPT;
    BuiltinFunction* args(std::initializer_list<const char8_t*> argList) SKR_NOEXCEPT;
};

} // namespace das
} // namespace skr

namespace skr {
namespace das {

template  <typename FuncT, FuncT fn, typename SimNodeT, typename FuncArgT>
class ExternalFn
{
public:
    FORCEINLINE ExternalFn(const char8_t* name, const Library& lib, const char8_t* cppName = nullptr)
    // : ExternalFnBase(name,cppName) 
    {
        // constructExternal(makeFuncArgs<FuncArgT>::make(lib));
    }
    FORCEINLINE ExternalFn(const char8_t* name, const char8_t* cppName = nullptr)
    // : ExternalFnBase(name,cppName)
    {
    }
protected:
    void* getBuiltinAddress() const 
    {
        return ImplWrapCall<SimNodeT::IS_CMRES, NeedVectorWrap<FuncT>::value, FuncT, fn>::get_builtin_address();
    }
};

} // namespace das
} // namespace skr