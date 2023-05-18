#pragma once
#include "misc/traits.hpp"
#include "platform/memory.h"
#include "serde/binary/writer_fwd.h"
#include "serde/binary/reader_fwd.h"
#include "serde/json/reader_fwd.h"
#include "serde/json/writer_fwd.h"

struct skr_value_t;

namespace skr
{
    template<class T>
    constexpr auto GetDefaultCtor() -> void(*)(void*, skr_value_t*, size_t)
    {
        if constexpr (std::is_default_constructible_v<T>)
            return [](void* self, skr_value_t* param, size_t nparam) 
            { 
                new (self) T();
            };
        return nullptr;
    }

    template<class T>
    constexpr auto GetDtor() -> void(*)(void*)
    {
        if constexpr (std::is_destructible_v<T>)
            return [](void* address) { ((T*)address)->~T(); };
        return nullptr;
    }

    template<class T>
    constexpr auto GetCopyCtor() -> void(*)(void*, const void*)
    {
        if constexpr (std::is_copy_constructible_v<T>)
            return [](void* address, const void* src) { new (address) T(*(T*)src); };
        return nullptr;
    }

    template<class T>
    constexpr auto GetMoveCtor() -> void(*)(void*, void*)
    {
        if constexpr (std::is_move_constructible_v<T>)
            return [](void* address, void* src) { new (address) T(std::move(*(T*)src)); };
        return nullptr;
    }

    

    template<class T>
    struct SerdeCompleteChecker : std::true_type {};

    template <class T>
    inline constexpr bool is_complete_serde()
    {
    if constexpr (skr::is_complete_v<T>)
    {
        return SerdeCompleteChecker<T>::value;
    }
    else
        return false;
    }

    template <class T>
    constexpr bool is_complete_serde_v = is_complete_serde<T>();

    template<class T>
    constexpr auto GetSerialize() -> int(*)(const void*, skr_binary_writer_t* writer)
    {
        if constexpr(is_complete_serde_v<skr::binary::WriteTrait<const T&>>)
            return [](const void* address, skr_binary_writer_t* archive) { 
                T* ptr = (T*)address;
                return skr::binary::WriteTrait<const T&>::Write(archive, *ptr);
            };
        return nullptr;
    }

    template<class T>
    constexpr auto GetDeserialize() -> int(*)(void*, skr_binary_reader_t* reader)
    {
        if constexpr(is_complete_serde_v<skr::binary::ReadTrait<T>>)
            return [](void* address, skr_binary_reader_t* archive) {
                T* ptr = (T*)address;
                return skr::binary::ReadTrait<T>::Read(archive, *ptr);
            };
        return nullptr;
    }

    template<class T>
    constexpr auto GetTextSerialize() -> void(*)(const void*, skr_json_writer_t* writer)
    {
        if constexpr(is_complete_serde_v<skr::json::WriteTrait<const T&>>)
            return [](const void* address, skr_json_writer_t* archive) {
                T* ptr = (T*)address;
                return skr::json::WriteTrait<const T&>::Write(archive, *ptr);
            };
        return nullptr;
    }

    template<class T>
    constexpr auto GetTextDeserialize() -> json::error_code(*)(void*, json::value_t&& reader)
    {
        if constexpr(is_complete_serde_v<skr::json::ReadTrait<T>>)
            return [](void* address, json::value_t&& archive) {
                T* ptr = (T*)address;
                return skr::json::ReadTrait<T>::Read(std::move(archive), *ptr);
            };
        return nullptr;
    }

    template<class T>
    constexpr auto GetDeleter() -> void(*)(void*)
    {
        if constexpr (std::is_destructible_v<T>)
            return [](void* address) { SkrDelete((T*)address); };
        return nullptr;
    }
}