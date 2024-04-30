#pragma once
#include <type_traits>

namespace skr::rttr
{
struct RTTRBackend {
    //======> Begin Export Backend API
    // ctor & dtor export
    template <typename T, typename... Args>
    static void* export_ctor();
    template <typename T>
    static void* export_dtor();

    // method export
    template <auto method>
    static void* export_method();
    template <auto method>
    static void* export_static_method();

    // field export
    template <auto field>
    static void* export_field_setter();
    template <auto field>
    static void* export_field_getter();

    // static field export
    template <auto field>
    static void* export_static_field_setter();
    template <auto field>
    static void* export_static_field_getter();

    // extern method export
    template <auto method>
    static void* export_extern_method();

    // function export
    template <auto func>
    static void* export_function();
    //======> End Export Backend API

    template <typename... Args>
    using CtorInvoker = void (*)(void*, Args...);
    using DtorInvoker = void (*)(void*);
    template <typename T, typename Ret, typename... Args>
    using MethodInvoker = Ret (*)(T*, Args...);
    template <typename T, typename Ret, typename... Args>
    using ConstMethodInvoker = Ret (*)(const T*, Args...);
    template <typename Ret, typename... Args>
    using FunctionInvoker = Ret (*)(Args...);

    // load ctor & dtor
    template <typename... Args>
    static CtorInvoker<Args...> load_ctor(void* invoker);
    static DtorInvoker          load_dtor(void* invoker);

    // load methods
    template <typename T, typename Ret, typename... Args>
    static MethodInvoker<T, Ret, Args...> load_method(void* invoker);
    template <typename T, typename Ret, typename... Args>
    static ConstMethodInvoker<T, Ret, Args...> load_method_const(void* invoker);

    // load extern methods
    template <typename Ret, typename... Args>
    static FunctionInvoker<Ret, Args...> load_extern_method(void* invoker);

    // load function
    template <typename Ret, typename... Args>
    static FunctionInvoker<Ret, Args...> load_function(void* invoker);

private:
    // helpers
    template <auto method, class T, typename Ret, typename... Args>
    static void* _make_method_proxy(Ret (T::*)(Args...));
    template <auto method, class T, typename Ret, typename... Args>
    static void* _make_method_proxy(Ret (T::*)(Args...) const);
};
} // namespace skr::rttr

namespace skr::rttr
{
// helpers
template <auto method, class T, typename Ret, typename... Args>
inline void* RTTRBackend::_make_method_proxy(Ret (T::*)(Args...))
{
    auto proxy = +[](void* obj, Args... args) -> Ret {
        return (reinterpret_cast<T*>(obj)->*method)(std::forward<Args>(args)...);
    };
    return reinterpret_cast<void*>(proxy);
}
template <auto method, class T, typename Ret, typename... Args>
inline void* RTTRBackend::_make_method_proxy(Ret (T::*)(Args...) const)
{
    auto proxy = +[](const void* obj, Args... args) -> Ret {
        return (reinterpret_cast<const T*>(obj)->*method)(std::forward<Args>(args)...);
    };
    return reinterpret_cast<void*>(proxy);
}

// ctor & dtor export
template <typename T, typename... Args>
inline void* RTTRBackend::export_ctor()
{
    auto result = +[](void* p, Args... args) {
        new (p) T(std::forward<Args>(args)...);
    };
    return reinterpret_cast<void*>(result);
}
template <typename T>
inline void* RTTRBackend::export_dtor()
{
    auto result = +[](void* p) {
        reinterpret_cast<T*>(p)->~T();
    };
    return reinterpret_cast<void*>(result);
}

// method export
template <auto method>
inline void* RTTRBackend::export_method()
{
    return _make_method_proxy<method>(method);
}
template <auto method>
inline void* RTTRBackend::export_static_method()
{
    return reinterpret_cast<void*>(method);
}

// field export
template <auto field>
inline void* RTTRBackend::export_field_setter()
{
    return nullptr;
}
template <auto field>
inline void* RTTRBackend::export_field_getter()
{
    return nullptr;
}

// static field export
template <auto field>
inline void* RTTRBackend::export_static_field_setter()
{
    return nullptr;
}
template <auto field>
inline void* RTTRBackend::export_static_field_getter()
{
    return nullptr;
}

// extern method export
template <auto method>
inline void* RTTRBackend::export_extern_method()
{
    return reinterpret_cast<void*>(method);
}

// function export
template <auto func>
inline void* RTTRBackend::export_function()
{
    return reinterpret_cast<void*>(func);
}

// load ctor & dtor
template <typename... Args>
inline RTTRBackend::CtorInvoker<Args...> RTTRBackend::load_ctor(void* invoker)
{
    return reinterpret_cast<CtorInvoker<Args...>>(invoker);
}
inline RTTRBackend::DtorInvoker RTTRBackend::load_dtor(void* invoker)
{
    return reinterpret_cast<DtorInvoker>(invoker);
}

// load methods
template <typename T, typename Ret, typename... Args>
inline RTTRBackend::MethodInvoker<T, Ret, Args...> RTTRBackend::load_method(void* invoker)
{
    return reinterpret_cast<MethodInvoker<T, Ret, Args...>>(invoker);
}
template <typename T, typename Ret, typename... Args>
inline RTTRBackend::ConstMethodInvoker<T, Ret, Args...> RTTRBackend::load_method_const(void* invoker)
{
    return reinterpret_cast<ConstMethodInvoker<T, Ret, Args...>>(invoker);
}

// load extern methods
template <typename Ret, typename... Args>
inline RTTRBackend::FunctionInvoker<Ret, Args...> RTTRBackend::load_extern_method(void* invoker)
{
    return reinterpret_cast<FunctionInvoker<Ret, Args...>>(invoker);
}

// load function
template <typename Ret, typename... Args>
inline RTTRBackend::FunctionInvoker<Ret, Args...> RTTRBackend::load_function(void* invoker)
{
    return reinterpret_cast<FunctionInvoker<Ret, Args...>>(invoker);
}

} // namespace skr::rttr
