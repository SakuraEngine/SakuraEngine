#pragma once
#include "SkrRT/platform/configure.h"
#include "EASTL/variant.h"

namespace skr
{
    template<class ...Ts>
    using variant = eastl::variant<Ts...>;
    template<class ...Ts>
    struct overload : Ts... { using Ts::operator()...; };
    template<class ...Ts>
    overload(Ts...) -> overload<Ts...>;
    using eastl::get_if;
    using eastl::get;
    using eastl::visit;
}

#include "SkrRT/type/type.hpp"

namespace skr
{
namespace type
{
struct VariantStorage
{
    size_t index;
    uint8_t data[1];
};
// skr::variant<Ts...>
struct VariantType : skr_type_t {
    const skr::span<const skr_type_t*> types;
    size_t size;
    size_t align;
    size_t padding;
    skr::string name;
    RUNTIME_API void Set(void* dst, size_t index, const void* src) const;
    RUNTIME_API void* Get(void* data, size_t index) const;
    RUNTIME_API size_t Index(void* data) const;
    VariantType(const skr::span<const skr_type_t*> types, size_t size, size_t align, size_t padding)
        : skr_type_t{ SKR_TYPE_CATEGORY_VARIANT }
        , types(types)
        , size(size)
        , align(align)
        , padding(padding)
    {
    }
};
RUNTIME_API const skr_type_t* make_variant_type(const skr::span<const skr_type_t*> types);
template <class... Ts>
struct type_of<skr::variant<Ts...>> {
    
    static const skr::span<const skr_type_t*> variants()
    {
        static const skr_type_t* datas[] = 
            { type_of<Ts>::get()... };
        return skr::span<const skr_type_t*>(datas);
    }
    static const skr_type_t* get()
    {
        static auto type = make_variant_type(variants());
        return type;
    }
};
} // namespace type
} // namespace skr

// binary reader
#include "SkrRT/serde/binary/reader_fwd.h"

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
#include "SkrRT/serde/binary/writer_fwd.h"

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
        eastl::visit([&](auto&& value) {
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