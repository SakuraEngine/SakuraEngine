#pragma once
#include <type_traits>
#include "SkrRTTR/export/export_data.hpp"

namespace skr::rttr
{
struct ExportHelper {
    // ctor & dtor export
    template <typename T, typename... Args>
    inline static void* export_ctor()
    {
        auto result = +[](void* p, Args... args) {
            new (p) T(std::forward<Args>(args)...);
        };
        return reinterpret_cast<void*>(result);
    }
    template <typename T>
    inline static void* export_dtor()
    {
        auto result = +[](void* p) {
            reinterpret_cast<T*>(p)->~T();
        };
        return reinterpret_cast<void*>(result);
    }

    // method export
    template <auto method>
    inline static void* export_method()
    {
        return _make_method_proxy<method>(method);
    }
    template <auto method>
    inline static void* export_static_method()
    {
        return reinterpret_cast<void*>(method);
    }

    // extern method export
    template <auto method>
    inline static void* export_extern_method()
    {
        return reinterpret_cast<void*>(method);
    }

    // function export
    template <auto func>
    inline static void* export_function()
    {
        return reinterpret_cast<void*>(func);
    }

    // invoker
    template <typename... Args>
    using CtorInvoker = void (*)(void*, Args...);
    using DtorInvoker = void (*)(void*);
    template <typename Ret, typename... Args>
    using MethodInvokerExpand = Ret (*)(void*, Args...);
    template <typename Ret, typename... Args>
    using ConstMethodInvokerExpand = Ret (*)(const void*, Args...);
    template <typename Ret, typename... Args>
    using FunctionInvokerExpand = Ret (*)(Args...);
    template <typename Func>
    struct ExpandInvoker {
    };
    template <typename Ret, typename... Args>
    struct ExpandInvoker<Ret (*)(Args...)> {
        using MethodInvoker      = MethodInvokerExpand<Ret, Args...>;
        using ConstMethodInvoker = ConstMethodInvokerExpand<Ret, Args...>;
        using FunctionInvoker    = FunctionInvokerExpand<Ret, Args...>;
    };
    template <typename Func>
    using MethodInvoker = typename ExpandInvoker<Func>::MethodInvoker;
    template <typename Func>
    using ConstMethodInvoker = typename ExpandInvoker<Func>::ConstMethodInvoker;
    template <typename Func>
    using FunctionInvoker = typename ExpandInvoker<Func>::FunctionInvoker;

private:
    // helpers
    template <auto method, class T, typename Ret, typename... Args>
    inline static void* _make_method_proxy(Ret (T::*)(Args...))
    {
        auto proxy = +[](void* obj, Args... args) -> Ret {
            return (reinterpret_cast<T*>(obj)->*method)(std::forward<Args>(args)...);
        };
        return reinterpret_cast<void*>(proxy);
    }
    template <auto method, class T, typename Ret, typename... Args>
    inline static void* _make_method_proxy(Ret (T::*)(Args...) const)
    {
        auto proxy = +[](const void* obj, Args... args) -> Ret {
            return (reinterpret_cast<const T*>(obj)->*method)(std::forward<Args>(args)...);
        };
        return reinterpret_cast<void*>(proxy);
    }
};
} // namespace skr::rttr
