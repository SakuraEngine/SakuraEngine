#pragma once
#include "platform/configure.h"
#include <variant>

namespace skr
{
    template<class ...Ts>
    using variant = std::variant<Ts...>;
    template<class ...Ts>
    struct overload : Ts... { using Ts::operator()...; };
    template<class ...Ts>
    overload(Ts...) -> overload<Ts...>;

    using std::get_if;
    using std::get;
}

#include "type/type.hpp"

namespace skr
{
namespace type
{
struct VariantMethodTable {
    const skr::span<void* (*)(void*)> getters;
    const skr::span<void (*)(void*, const void*)> setters;
    size_t (*indexer)(const void* self);
    void (*dtor)(void* self);
    void (*ctor)(void* self, Value* param, size_t nparam);
    void (*copy)(void* self, const void* other);
    void (*move)(void* self, void* other);
    void (*deleter)(void*);
    int (*Serialize)(const void* self, skr_binary_writer_t* writer);
    int (*Deserialize)(void* self, skr_binary_reader_t* reader);
    void (*SerializeText)(const void*, skr_json_writer_t* writer);
    json::error_code (*DeserializeText)(void* self, json::value_t&& reader);
};

// skr::variant<Ts...>
struct VariantType : skr_type_t {
    const skr::span<const skr_type_t*> types;
    size_t size;
    size_t align;
    skr::text::text name;
    VariantMethodTable operations;
    VariantType(const skr::span<const skr_type_t*> types, size_t size, size_t align,
    VariantMethodTable operations)
        : skr_type_t{ SKR_TYPE_CATEGORY_VARIANT }
        , types(types)
        , size(size)
        , align(align)
        , operations(operations)
    {
    }
};

template <class... Ts>
struct type_of<skr::variant<Ts...>> {
    static const skr::span<const skr_type_t*> variants()
    {
        static const skr_type_t* datas[] = 
            { type_of<Ts>::get()... };
        return skr::span<const skr_type_t*>(datas);
    }
    template<size_t I>
    static auto getter() -> void* (*)(void*)
    {
        return +[](void* data)->void*
        {
            return &std::get<I>(*(skr::variant<Ts...>*)data);
        };
    }
    template<size_t I>
    static auto setter() -> void (*)(void*, const void*)
    {
        return +[](void* data, const void* value)
        {
            using T = std::variant_alternative_t<I, skr::variant<Ts...>>;
            if(value != nullptr)
                *(skr::variant<Ts...>*)data = *(const T*)value;
            else
                *(skr::variant<Ts...>*)data = T();
        };
    }
    template<size_t... Is>
    static auto getters(std::index_sequence<Is...>) -> skr::span<void* (*)(void*)>
    {
        static void* (*getters[])(void*) = { getter<Is>()... };
        return skr::span<void* (*)(void*)>(getters);
    }
    template<size_t... Is>
    static auto setters(std::index_sequence<Is...>) -> skr::span<void (*)(void*, const void*)>
    {
        static void (*setters[])(void*, const void*) = { setter<Is>()... };
        return skr::span<void (*)(void*, const void*)>(setters);
    }
    static auto indexer()
    {
        return +[](const void* data)->size_t
        {
            return ((const skr::variant<Ts...>*)data)->index();
        };
    }
    static const skr_type_t* get()
    {
        using V = skr::variant<Ts...>;
        VariantMethodTable op{
            getters(std::make_index_sequence<sizeof...(Ts)>()),
            setters(std::make_index_sequence<sizeof...(Ts)>()),
            indexer(),
            GetDtor<V>(),
            GetDefaultCtor<V>(),
            GetCopyCtor<V>(),
            GetMoveCtor<V>(),
            GetDeleter<V>(),
            GetSerialize<V>(),
            GetDeserialize<V>(),
            GetTextSerialize<V>(),
            GetTextDeserialize<V>(),
        };
        static VariantType type{
            variants(),
            sizeof(skr::variant<Ts...>),
            alignof(skr::variant<Ts...>),
            op
        };
        return &type;
    }
};
} // namespace type
} // namespace skr

// binary reader
#include "binary/reader_fwd.h"

namespace skr
{
namespace binary
{
template<class ...Ts>
struct ReadTrait<skr::variant<Ts...>>
{
    template<size_t I, class T>
    static int ReadByIndex(skr_binary_reader_t* archive, skr::variant<Ts...>& value, size_t index)
    {
        if (index == I)
        {
            T t;
            SKR_ARCHIVE(t);
            value = std::move(t);
            return 0;
        }
        return -1;
    }

    template<size_t ...Is>
    static int ReadByIndexHelper(skr_binary_reader_t* archive, skr::variant<Ts...>& value, size_t index, std::index_sequence<Is...>)
    {
        int result;
        (void)(((result = ReadByIndex<Is, Ts>(archive, value, index)) != 0) && ...);
        return result;
    }

    static int Read(skr_binary_reader_t* archive, skr::variant<Ts...>& value)
    {
        uint32_t index;
        SKR_ARCHIVE(index);
        if (index >= sizeof...(Ts))
            return -1;
        return ReadByIndexHelper(archive, value, index, std::make_index_sequence<sizeof...(Ts)>());
    }
};

} // namespace binary

template <class ...Ts>
struct SerdeCompleteChecker<binary::ReadTrait<skr::variant<Ts...>>>
    : std::bool_constant<(is_complete_serde_v<binary::ReadTrait<Ts>> && ...)> {
};
} // namespace skr

// binary writer
#include "binary/writer_fwd.h"

namespace skr
{
namespace binary
{
template <class... Ts>
struct WriteTrait<const skr::variant<Ts...>&> {
    static int Write(skr_binary_writer_t* archive, const skr::variant<Ts...>& variant)
    {
        SKR_ARCHIVE((uint32_t)variant.index());
        int ret;
        std::visit([&](auto&& value) {
            ret = skr::binary::Archive(archive, value);
        },
        variant);
        return ret;
    }
};
} // namespace binary
template <class... Ts>
struct SerdeCompleteChecker<binary::WriteTrait<const skr::variant<Ts...>&>>
    : std::bool_constant<(is_complete_serde_v<json::WriteTrait<Ts>> && ...)> {
};
} // namespace skr