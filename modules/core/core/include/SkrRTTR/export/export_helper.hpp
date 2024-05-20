#pragma once
#include <type_traits>
#include "SkrRTTR/export/export_data.hpp"
#include "SkrRTTR/export/stack_proxy.hpp"

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
    template <typename T, typename... Args>
    inline static MethodInvokerStackProxy export_ctor_stack_proxy()
    {
        return _make_ctor_stack_proxy<T, Args...>(std::make_index_sequence<sizeof...(Args)>());
    }
    template <typename T>
    inline static DtorInvoker export_dtor()
    {
        auto result = +[](void* p) {
            reinterpret_cast<T*>(p)->~T();
        };
        return result;
    }

    // method export
    template <auto method>
    inline static void* export_method()
    {
        return _make_method_proxy<method>(method);
    }
    template <auto method>
    inline static MethodInvokerStackProxy export_method_stack_proxy()
    {
        return _make_method_stack_proxy<method>(method);
    }
    template <auto method>
    inline static void* export_static_method()
    {
        return reinterpret_cast<void*>(method);
    }
    template <auto method>
    inline static FuncInvokerStackProxy export_static_method_stack_proxy()
    {
        return _make_function_stack_proxy<method>(method);
    }

    // extern method export
    template <auto method>
    inline static void* export_extern_method()
    {
        return reinterpret_cast<void*>(method);
    }
    template <auto method>
    inline static FuncInvokerStackProxy export_extern_method_stack_proxy()
    {
        return _make_function_stack_proxy<method>(method);
    }

    // function export
    template <auto func>
    inline static void* export_function()
    {
        return reinterpret_cast<void*>(func);
    }
    template <auto func>
    inline static FuncInvokerStackProxy export_function_stack_proxy()
    {
        return _make_function_stack_proxy<func>(func);
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
    template <typename T, typename... Args, size_t... Idx>
    inline static MethodInvokerStackProxy _make_ctor_stack_proxy(std::index_sequence<Idx...>)
    {
        return +[](void* p, StackProxy proxy) {
            std::tuple<ParamHolder<Args>...> tuples(proxy.param_builders[Idx]...);
            new (p) T(std::forward<Args>(std::get<Idx>(tuples).get())...);
        };
    }
    template <auto func, typename Ret, typename... Args>
    inline static FuncInvokerStackProxy _make_function_stack_proxy(Ret (*)(Args...))
    {
        return _make_function_stack_proxy_helper<func, Ret, Args...>(std::make_index_sequence<sizeof...(Args)>());
    }
    template <auto func, typename Ret, typename... Args, size_t... Idx>
    inline static FuncInvokerStackProxy _make_function_stack_proxy_helper(std::index_sequence<Idx...>)
    {
        return +[](StackProxy proxy) {
            std::tuple<ParamHolder<Args>...> tuples(proxy.param_builders[Idx]...);
            func(std::forward<Args>(std::get<Idx>(tuples).get())...);
        };
    }
    template <auto method, typename T, typename Ret, typename... Args>
    inline static MethodInvokerStackProxy _make_method_stack_proxy(Ret (T::*)(Args...))
    {
        return _make_method_stack_proxy_helper<method, T, Ret, Args...>(std::make_index_sequence<sizeof...(Args)>());
    }
    template <auto method, typename T, typename Ret, typename... Args>
    inline static MethodInvokerStackProxy _make_method_stack_proxy(Ret (T::*)(Args...) const)
    {
        return _make_method_stack_proxy_helper_const<method, T, Ret, Args...>(std::make_index_sequence<sizeof...(Args)>());
    }
    template <auto method, typename T, typename Ret, typename... Args, size_t... Idx>
    inline static MethodInvokerStackProxy _make_method_stack_proxy_helper(std::index_sequence<Idx...>)
    {
        return +[](void* p, StackProxy proxy) {
            std::tuple<ParamHolder<Args>...> tuples(proxy.param_builders[Idx]...);
            (reinterpret_cast<T*>(p)->*method)(std::forward<Args>(std::get<Idx>(tuples).get())...);
        };
    }
    template <auto method, typename T, typename Ret, typename... Args, size_t... Idx>
    inline static MethodInvokerStackProxy _make_method_stack_proxy_helper_const(std::index_sequence<Idx...>)
    {
        return +[](const void* p, StackProxy proxy) {
            std::tuple<ParamHolder<Args>...> tuples(proxy.param_builders[Idx]...);
            (reinterpret_cast<const T*>(p)->*method)(std::forward<Args>(std::get<Idx>(tuples).get())...);
        };
    }
};
} // namespace skr::rttr
