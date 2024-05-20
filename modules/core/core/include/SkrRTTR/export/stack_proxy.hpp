#pragma once
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrContainers/function_ref.hpp"
#include "SkrContainers/span.hpp"

namespace skr::rttr
{
enum class EParamHolderType
{
    value,  // 一对一的栈值
    xvalue, // 将亡值
};

// param builder
using BuildParamFunc    = EParamHolderType(void* data, uint64_t size, uint64_t align);
using BuildParamFuncRef = FunctionRef<BuildParamFunc>;

template <typename T>
struct ParamHolder {
    inline ParamHolder(BuildParamFuncRef builder)
    {
        auto type = builder(_holder.data(), sizeof(T), alignof(T));
        SKR_ASSERT(type == EParamHolderType::value && "type not reference cannot has xvalue");
    }

    inline ~ParamHolder()
    {
        _holder.data_typed()->~T();
    }

    inline T& get()
    {
        return *_holder.data_typed();
    }

private:
    Placeholder<T> _holder;
};
template <typename T>
struct ParamHolder<T&> {
    inline ParamHolder(BuildParamFuncRef builder)
    {
        auto type = builder(_xvalue_holder.data(), sizeof(T), alignof(T));
        SKR_ASSERT(type == EParamHolderType::value && "type not reference cannot has xvalue");
    }

    inline ~ParamHolder()
    {
        if (_type == EParamHolderType::xvalue)
        {
            _xvalue_holder.data_typed()->~T();
        }
    }

    inline T& get()
    {
        switch (_type)
        {
            case EParamHolderType::value:
                return *reinterpret_cast<T*>(_value);
            case EParamHolderType::xvalue:
                return *_xvalue_holder.data_typed();
            default:
                SKR_UNREACHABLE_CODE()
                return *reinterpret_cast<T*>(_value);
        }
    }

private:
    EParamHolderType _type;
    union
    {
        void*          _value;
        Placeholder<T> _xvalue_holder;
    };
};

// ignore cv
template <typename T>
struct ParamHolder<const T> : ParamHolder<T> {
};
template <typename T>
struct ParamHolder<volatile T> : ParamHolder<T> {
};

// stack proxy
struct ReturnHolder {
    void*    data;
    uint64_t size;
    uint64_t align;
};
struct StackProxy {
    ReturnHolder            return_holder;
    span<BuildParamFuncRef> param_builders;
};

// proxy invoker
using MethodInvokerStackProxy = void (*)(void* p, StackProxy proxy);
using FuncInvokerStackProxy   = void (*)(StackProxy proxy);
} // namespace skr::rttr