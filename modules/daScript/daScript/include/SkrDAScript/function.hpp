#pragma once
#include "SkrDAScript/type.hpp"
#include <EASTL/array.h>

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

typedef skr::das::reg4f (*BuiltInSimProc)(::das::SimNode**, ::das::Context*, void*);
struct SKR_DASCRIPT_API BuiltInFunction
{
    static BuiltInFunction MakeExternFunction(const Library* lib, 
    BuiltInSimProc call, bool IS_CMRES, const char8_t* name, const char8_t* cppName = nullptr) SKR_NOEXCEPT;

    template <typename FuncT, FuncT fn, bool IS_REF, bool IS_CMRES = false, typename FuncArgT = FuncT>
    static BuiltInFunction MakeExternFunction(const Library* lib, const char8_t* name, const char8_t* cppName = nullptr) SKR_NOEXCEPT;

    void set_is_call_based(bool) SKR_NOEXCEPT;
    bool is_call_based() const SKR_NOEXCEPT;
    void set_is_property(bool) SKR_NOEXCEPT;
    bool is_property() const SKR_NOEXCEPT;

    void construct(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT;
    void construct_external(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT;
    void construct_interop(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT;

    BuiltInFunction& arg(const char8_t* argName) SKR_NOEXCEPT;
    inline BuiltInFunction& args(std::initializer_list<const char8_t*> argList) SKR_NOEXCEPT
    {
        for (auto argName : argList)
        {
            arg(argName);
        }
        return *this;
    }
public:
    BuiltInFunction(const BuiltInFunction& t) SKR_NOEXCEPT;
    BuiltInFunction& operator=(const BuiltInFunction& t) SKR_NOEXCEPT;

    BuiltInFunction() SKR_NOEXCEPT;
    BuiltInFunction(std::nullptr_t) SKR_NOEXCEPT;
    ~BuiltInFunction() SKR_NOEXCEPT;

public: // none-export methods
    static BuiltInFunction _make(::das::BuiltInFunction* ptr);
    ::das::BuiltInFunction* _get() const;

private:
    BuiltInFunction(::das::BuiltInFunction* ptr) : ptr(ptr) {}
    ::das::BuiltInFunction* ptr = nullptr;
};

template <typename RetT, size_t N, typename ...Args>
FORCEINLINE eastl::array<TypeDecl, N> make_builtin_args(const Library* lib)
{
    eastl::array<TypeDecl, N> args = { TypeDecl::MakeType<RetT>(lib), TypeDecl::MakeArgumentType<Args>(lib)... };
    return std::move(args);
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
    static FORCEINLINE eastl::array<TypeDecl, 1 + sizeof...(Args)> make(const Library* lib) 
    {
        return std::move(make_builtin_args<R, 1 + sizeof...(Args), Args...>(lib));
    }
};

template <typename FuncT, FuncT fn, bool IS_REF, bool IS_CMRES, typename FuncArgT>
inline BuiltInFunction BuiltInFunction::MakeExternFunction(const Library* lib, const char8_t* name, const char8_t* cppName) SKR_NOEXCEPT
{
    const auto args = make_func_args<FuncArgT>::make(lib);
    BuiltInSimProc call = +[](::das::SimNode** args, ::das::Context* context, void* res) -> skr::das::reg4f 
    {
        // auto addr = ImplWrapCall<IS_CMRES, NeedVectorWrap<FuncT>::value, FuncT, fn>::get_builtin_address();
        if constexpr (IS_CMRES)
        {
            return ImplCallStaticFunctionAndCopy<FuncT>::call(*fn, *context, res, args);
        }
        else if constexpr (IS_REF)
        {
            return ImplCallStaticFunctionRef<FuncT>::call(*fn, *context, args);
        }
        else
        {
            return ImplCallStaticFunction<FuncT>::call(*fn, *context, args);
        }
    };
    auto builtin = BuiltInFunction::MakeExternFunction(lib, call, IS_CMRES, name, cppName);
    builtin.construct_external(args.data(), args.size());
    return builtin;
}

} // namespace das
} // namespace skr