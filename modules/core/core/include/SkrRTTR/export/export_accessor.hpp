#pragma once
#include <SkrRTTR/export/export_data.hpp>

// invoker
namespace skr
{
template <typename Func>
struct ExportCtorInvoker;
template <typename Func>
struct ExportMethodInvoker;
template <typename Func>
struct ExportStaticMethodInvoker;
template <typename Func>
struct ExportExternMethodInvoker;

template <typename... Args>
struct ExportCtorInvoker<void(Args...)> {
    inline ExportCtorInvoker() = default;
    inline ExportCtorInvoker(const RTTRCtorData* ctor_data)
    {
        _ctor_data = ctor_data;
    }

    // getter
    inline const RTTRCtorData* data() const { return _ctor_data; }
    inline bool                is_valid() const { return _ctor_data != nullptr; }
    inline                     operator bool() const { return is_valid(); }

    // invoke
    inline void invoke(void* p_obj, Args... args) const
    {
        SKR_ASSERT(_ctor_data->native_invoke);
        auto invoker = reinterpret_cast<void (*)(void*, Args...)>(_ctor_data->native_invoke);
        invoker(p_obj, args...);
    }

private:
    const RTTRCtorData* _ctor_data = nullptr;
};
template <typename Ret, typename... Args>
struct ExportMethodInvoker<Ret(Args...)> {
    inline ExportMethodInvoker() = default;
    inline ExportMethodInvoker(const RTTRMethodData* method_data)
    {
        _method_data = method_data;
    }

    // getter
    inline const RTTRMethodData* data() const { return _method_data; }
    inline bool                  is_valid() const { return _method_data != nullptr; }
    inline                       operator bool() const { return is_valid(); }

    // invoke
    inline Ret invoke(void* p_obj, Args... args) const
    {
        SKR_ASSERT(_method_data->native_invoke);
        auto invoker = reinterpret_cast<Ret (*)(void*, Args...)>(_method_data->native_invoke);
        return invoker(p_obj, args...);
    }

private:
    const RTTRMethodData* _method_data = nullptr;
};
template <typename Ret, typename... Args>
struct ExportStaticMethodInvoker<Ret(Args...)> {
    inline ExportStaticMethodInvoker() = default;
    inline ExportStaticMethodInvoker(const RTTRStaticMethodData* method_data)
    {
        _method_data = method_data;
    }

    // getter
    inline const RTTRStaticMethodData* data() const { return _method_data; }
    inline bool                        is_valid() const { return _method_data != nullptr; }
    inline                             operator bool() const { return is_valid(); }

    // invoke
    inline Ret invoke(Args... args) const
    {
        SKR_ASSERT(_method_data->native_invoke);
        auto invoker = reinterpret_cast<Ret (*)(Args...)>(_method_data->native_invoke);
        return invoker(args...);
    }

private:
    const RTTRStaticMethodData* _method_data = nullptr;
};
template <typename Ret, typename... Args>
struct ExportExternMethodInvoker<Ret(Args...)> {
    inline ExportExternMethodInvoker() = default;
    inline ExportExternMethodInvoker(const RTTRExternMethodData* method_data)
    {
        _method_data = method_data;
    }

    // getter
    inline const RTTRExternMethodData* data() const { return _method_data; }
    inline bool                        is_valid() const { return _method_data != nullptr; }
    inline                             operator bool() const { return is_valid(); }

    // invoke
    inline Ret invoke(Args... args) const
    {
        SKR_ASSERT(_method_data->native_invoke);
        auto invoker = reinterpret_cast<Ret (*)(Args...)>(_method_data->native_invoke);
        return invoker(args...);
    }

private:
    const RTTRExternMethodData* _method_data = nullptr;
};
} // namespace skr

// visitor
namespace skr
{
template <typename T>
struct ExportFieldVisitor {
};
template <typename T>
struct ExportStaticFieldVisitor {
};
} // namespace skr