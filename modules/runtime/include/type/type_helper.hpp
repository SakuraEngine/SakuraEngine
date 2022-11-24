#pragma once
#include "platform/memory.h"
#include "binary/writer.h"
#include "binary/reader.h"

struct skr_value_t;

namespace skr
{
    template<class T, class = void>
    struct is_complete : std::false_type {};

    template<class T>
    struct is_complete<T, std::enable_if_t<sizeof(T)>> : std::true_type {};

    template<class T>
    constexpr bool is_complete_v = is_complete<T>::value;

    template<class T>
    auto GetDefaultCtor() -> void(*)(void*, skr_value_t*, size_t)
    {
        return [](void* self, skr_value_t* param, size_t nparam) 
        { 
            new (self) T();
        };
    }

    template<class T>
    auto GetDtor() -> void(*)(void*)
    {
        if constexpr (std::is_destructible_v<T>)
            return [](void* address) { ((T*)address)->~T(); };
        return nullptr;
    }

    template<class T>
    auto GetCopyCtor() -> void(*)(void*, const void*)
    {
        if constexpr (std::is_copy_constructible_v<T>)
            return [](void* address, const void* src) { new (address) T(*(T*)src); };
        return nullptr;
    }

    template<class T>
    auto GetMoveCtor() -> void(*)(void*, void*)
    {
        if constexpr (std::is_move_constructible_v<T>)
            return [](void* address, void* src) { new (address) T(std::move(*(T*)src)); };
        return nullptr;
    }

    template<class T>
    auto GetSerialize() -> int(*)(const void*, skr_binary_writer_t* writer)
    {
        using TType = skr::binary::TParamType<T>;
        if constexpr(is_complete_v<skr::binary::WriteHelper<TType>>)
            return [](const void* address, skr_binary_writer_t* archive) { 
                T* ptr = (T*)address;
                return skr::binary::WriteHelper<TType>::Write(archive, *ptr);
            };
        return nullptr;
    }

    template<class T>
    auto GetDeserialize() -> int(*)(void*, skr_binary_reader_t* reader)
    {
        if constexpr(is_complete_v<skr::binary::ReadHelper<T>>)
            return [](void* address, skr_binary_reader_t* archive) {
                T* ptr = (T*)address;
                return skr::binary::ReadHelper<T>::Read(archive, *ptr);
            };
        return nullptr;
    }

    template<class T>
    auto GetDeleter() -> void(*)(void*)
    {
        if constexpr (std::is_destructible_v<T>)
            return [](void* address) { SkrDelete((T*)address); };
        return nullptr;
    }
}