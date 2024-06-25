#pragma once
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrContainers/function_ref.hpp"
#include "SkrContainers/span.hpp"
// TODO. 使用开堆的方式实现，并且通过两个数组，一个存储参数的存储方式和偏移，一个存储参数的具体数据，这样可以保持冗余，比如 String -> StringView

namespace skr::rttr
{
enum class EParamHolderType
{
    value,  // 一对一的栈值
    xvalue, // 将亡值
};

// param builder
using ParamWriteFunc    = EParamHolderType(void* data, uint64_t size, uint64_t align);
using ParamReadFunc     = void(void* data, uint64_t size, uint64_t align, EParamHolderType type);
using RetReadFunc       = void(void* data, uint64_t size, uint64_t align);
using ParamWriteFuncRef = FunctionRef<ParamWriteFunc>;
using ParamReadFuncRef  = FunctionRef<ParamReadFunc>;
using RetReadFuncRef    = FunctionRef<RetReadFunc>;

// TODO. ret 导出
// TODO. param 是 out 的时候怎么导出
// param holder
struct ParamProxy {
    ParamWriteFuncRef writer = nullptr;
    ParamReadFuncRef  reader = nullptr;
};
template <typename T>
struct ParamHolder {
    inline ParamHolder(ParamWriteFuncRef builder)
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

    inline void read(ParamReadFuncRef reader)
    {
        reader(_holder.data(), sizeof(T), alignof(T), EParamHolderType::value);
    }

private:
    Placeholder<T> _holder;
};
template <typename T>
struct ParamHolder<T&> {
    inline ParamHolder(ParamWriteFuncRef writer)
    {
        auto type = writer(_xvalue_holder.data(), sizeof(T), alignof(T));
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

    inline void read(ParamReadFuncRef reader)
    {
        switch (_type)
        {
            case EParamHolderType::value:
                reader(_value, sizeof(T), alignof(T), EParamHolderType::value);
                break;
            case EParamHolderType::xvalue:
                reader(_xvalue_holder.data(), sizeof(T), alignof(T), EParamHolderType::xvalue);
                break;
            default:
                SKR_UNREACHABLE_CODE()
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
template <typename T>
struct ParamHolder<T&&> {
    inline ParamHolder(ParamWriteFuncRef writer)
    {
        auto type = writer(_xvalue_holder.data(), sizeof(T), alignof(T));
        SKR_ASSERT(type == EParamHolderType::value && "type not reference cannot has xvalue");
    }

    inline ~ParamHolder()
    {
        if (_type == EParamHolderType::xvalue)
        {
            _xvalue_holder.data_typed()->~T();
        }
    }

    inline T&& get()
    {
        switch (_type)
        {
            case EParamHolderType::value:
                return std::move(*reinterpret_cast<T*>(_value));
            case EParamHolderType::xvalue:
                return std::move(*_xvalue_holder.data_typed());
            default:
                SKR_UNREACHABLE_CODE()
                return std::move(*reinterpret_cast<T*>(_value));
        }
    }

    inline void read(ParamReadFuncRef reader)
    {
        switch (_type)
        {
            case EParamHolderType::value:
                reader(_value, sizeof(T), alignof(T), EParamHolderType::value);
                break;
            case EParamHolderType::xvalue:
                reader(_xvalue_holder.data(), sizeof(T), alignof(T), EParamHolderType::xvalue);
                break;
            default:
                SKR_UNREACHABLE_CODE()
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

// return holder
template <typename T>
struct RetHolder {
    inline RetHolder(T holder)
        : _holder(holder)
    {
    }

    inline void read(RetReadFuncRef reader)
    {
        reader(this, sizeof(T), alignof(T));
    }

private:
    T _holder;
};

// stack proxy
struct StackProxy {
    RetReadFuncRef   ret_reader     = nullptr;
    span<ParamProxy> param_builders = {};
};

// proxy invoker
using MethodInvokerStackProxy = void (*)(void* p, StackProxy proxy);
using FuncInvokerStackProxy   = void (*)(StackProxy proxy);
} // namespace skr::rttr