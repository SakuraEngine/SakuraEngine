#pragma once
#include "SkrDAScript/type.hpp"
#include <EASTL/fixed_vector.h>

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

using BuiltInSimProc = void(*)();
struct BuiltInFunction
{
    SKR_DASCRIPT_API static BuiltInFunction MakeExternFunction(const Library* lib, 
        BuiltInSimProc call, bool IS_CMRES, const char8_t* name, const char8_t* cppName = nullptr) SKR_NOEXCEPT;

    template <typename FuncT, FuncT fn, bool IS_CMRES = false, typename FuncArgT = FuncT>
    static BuiltInFunction MakeExternFunction(const Library* lib, const char8_t* name, const char8_t* cppName = nullptr) SKR_NOEXCEPT;

    SKR_DASCRIPT_API void set_is_call_based(bool) SKR_NOEXCEPT;
    SKR_DASCRIPT_API bool is_call_based() const SKR_NOEXCEPT;
    SKR_DASCRIPT_API void set_is_property(bool) SKR_NOEXCEPT;
    SKR_DASCRIPT_API bool is_property() const SKR_NOEXCEPT;

    SKR_DASCRIPT_API void construct(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT;
    SKR_DASCRIPT_API void construct_external(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT;
    SKR_DASCRIPT_API void construct_interop(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT;

    SKR_DASCRIPT_API BuiltInFunction& arg(const char8_t* argName) SKR_NOEXCEPT;
    inline BuiltInFunction& args(std::initializer_list<const char8_t*> argList) SKR_NOEXCEPT
    {
        for (auto argName : argList)
            arg(argName);
        return *this;
    }
public:
    SKR_DASCRIPT_API BuiltInFunction() SKR_NOEXCEPT;
    SKR_DASCRIPT_API BuiltInFunction(std::nullptr_t) SKR_NOEXCEPT;
    SKR_DASCRIPT_API ~BuiltInFunction() SKR_NOEXCEPT;

public: // none-export methods
    static BuiltInFunction _make(::das::BuiltInFunction* ptr);
    ::das::BuiltInFunction* _get() const;

private:
    BuiltInFunction(::das::BuiltInFunction* ptr) : ptr(ptr) {}
    ::das::BuiltInFunction* ptr = nullptr;
};

template <typename RetT, typename ...Args>
FORCEINLINE std::initializer_list<TypeDecl> make_builtin_args(const Library* lib) 
{
    return { TypeDecl::MakeType<RetT>(lib), TypeDecl::MakeArgumentType<Args>(lib)... };
}

template<typename F> struct make_func_args;
template<typename R, typename ...Args> 
struct make_func_args<R(*)(Args...)> : make_func_args<R (Args...)> {};

} // namespace das
} // namespace skr

namespace skr {
namespace das {

template<typename R, typename ...Args>
struct make_func_args<R(Args...)> 
{
    static FORCEINLINE std::initializer_list<TypeDecl> make(const Library* lib) 
    {
        return make_builtin_args<R, Args...>(lib);
    }
};

template <typename FuncT, FuncT fn, bool IS_CMRES, typename FuncArgT>
BuiltInFunction BuiltInFunction::MakeExternFunction(const Library* lib, const char8_t* name, const char8_t* cppName) SKR_NOEXCEPT
{
    eastl::fixed_vector<skr::das::TypeDecl, 8> args = make_func_args<FuncArgT>::make(lib);
    BuiltInSimProc call = +[]() {
        auto addr = ImplWrapCall<IS_CMRES, NeedVectorWrap<FuncT>::value, FuncT, fn>::get_builtin_address();
        if constexpr (IS_CMRES)
        {

        }
        else
        {

        }
    };
    auto builtin = BuiltInFunction::MakeExternFunction(lib, call, IS_CMRES, name, cppName);
    builtin.construct_external(args.data(), args.size());
    return builtin;
}

} // namespace das
} // namespace skr