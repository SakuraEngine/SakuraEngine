#pragma once
#include <SkrRTTR/type_signature.hpp>

namespace skr
{
struct StackProxy {
    void*         data      = nullptr;
    TypeSignature signature = {};
};
template <typename T>
struct StackProxyMaker {
    inline static StackProxy Make(T& data, bool with_signature = true)
    {
        return {
            .data      = &data,
            .signature = with_signature ? type_signature_of<T>() : TypeSignature{},
        };
    }
    inline static StackProxy Make(T&& data, bool with_signature = true)
    {
        return {
            .data      = &data,
            .signature = with_signature ? type_signature_of<T>() : TypeSignature{},
        };
    }
};
template <typename T>
struct StackProxyMaker<T*> { // see object export, we only pass object by pointer, and we should change pointer value when return it
    inline static StackProxy Make(T*& data, bool with_signature = true)
    {
        return {
            .data      = &data,
            .signature = with_signature ? type_signature_of<T>() : TypeSignature{},
        };
    }
};
template <typename T>
struct StackProxyMaker<const T*> { // see object export, we only pass object by pointer, and we should change pointer value when return it
    inline static StackProxy Make(const T*& data, bool with_signature = true)
    {
        return {
            .data      = &data,
            .signature = with_signature ? type_signature_of<T>() : TypeSignature{},
        };
    }
};
template <typename T>
struct StackProxyMaker<T&> {
    inline static StackProxy Make(T& data, bool with_signature = true)
    {
        return {
            .data      = &data,
            .signature = with_signature ? type_signature_of<T>() : TypeSignature{},
        };
    }
};
template <typename T>
struct StackProxyMaker<const T&> {
    inline static StackProxy Make(const T& data, bool with_signature = true)
    {
        return {
            .data      = const_cast<T*>(&data),
            .signature = with_signature ? type_signature_of<T>() : TypeSignature{},
        };
    }
};
template <typename T>
struct StackProxyMaker<T&&> {
    inline static StackProxy Make(T&& data, bool with_signature = true)
    {
        return {
            .data      = &data,
            .signature = with_signature ? type_signature_of<T>() : TypeSignature{},
        };
    }
};

} // namespace skr