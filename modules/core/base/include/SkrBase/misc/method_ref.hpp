#pragma once
#include <type_traits>

namespace skr
{
struct MethodRef {
    template <auto method>
    inline void bind()
    {
        _proxy = _make_proxy<method>(method);
    }

    template <typename Ret, class T, typename... Args>
    inline Ret invoke(const T* obj, Args&&... args)
    {
        auto proxy = reinterpret_cast<Ret (*)(const T*, Args...)>(_proxy);
        return proxy(obj, std::forward<Args>(args)...);
    }

private:
    template <auto method, class T, typename Ret, typename... Args>
    inline static void* _make_proxy(Ret (T::*)(Args...))
    {
        auto proxy = +[](void* obj, Args... args) -> Ret {
            return (reinterpret_cast<T*>(obj)->*method)(std::forward<Args>(args)...);
        };
        return reinterpret_cast<void*>(proxy);
    }

    template <auto method, class T, typename Ret, typename... Args>
    inline static void* _make_proxy(Ret (T::*)(Args...) const)
    {
        auto proxy = +[](const void* obj, Args... args) -> Ret {
            return (reinterpret_cast<const T*>(obj)->*method)(std::forward<Args>(args)...);
        };
        return reinterpret_cast<void*>(proxy);
    }

private:
    void* _proxy;
};
} // namespace skr