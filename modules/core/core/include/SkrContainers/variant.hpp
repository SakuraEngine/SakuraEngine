#pragma once
#include "SkrContainersDef/variant.hpp"

// bin serde
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <class... Ts>
struct BinSerde<skr::variant<Ts...>> {
    template <size_t I, class T>
    inline static bool _read_by_index(SBinaryReader* r, skr::variant<Ts...>& v, size_t index)
    {
        if (index == I)
        {
            T tmp;
            if (!bin_read(r, (tmp))) return false;
            v = std::move(tmp);
            return true;
        }
        return false;
    }
    template <size_t... Is>
    inline static bool _read_by_index_helper(SBinaryReader* r, skr::variant<Ts...>& v, size_t index, std::index_sequence<Is...>)
    {
        bool result;
        (void)(((result = _read_by_index<Is, Ts>(r, v, index)) != 0) && ...);
        return result;
    }

    inline static bool read(SBinaryReader* r, skr::variant<Ts...>& v)
    {
        using SizeType = decltype(v.index());

        // read index
        SizeType index;
        if (!bin_read(r, index)) return false;
        if (index >= sizeof...(Ts))
            return false;

        // read content
        return _read_by_index(r, v, index, std::make_index_sequence<sizeof...(Ts)>());
    }

    inline static int write(SBinaryWriter* r, const skr::variant<Ts...>& v)
    {
        // write index
        if (!bin_write(r, v.index())) return false;

        // write content
        bool ret;
        skr::visit([&](auto&& value) {
            ret = bin_write(r, value);
        },
                   v);

        return ret;
    }
};
} // namespace skr

// json serde
#include "SkrSerde/json_serde.hpp"
#include "SkrRTTR/rttr_traits.hpp"
namespace skr
{
template <class... Ts>
struct JsonSerde<skr::variant<Ts...>> {
    template <class T>
    inline static bool _read_by_index(skr::archive::JsonReader* r, skr::variant<Ts...>& v, skr_guid_t index)
    {
        if (index == ::skr::rttr::type_id_of<T>())
        {
            T t;
            if (!json_read(r, t))
                return false;
            v = std::move(t);
            return true;
        }
        return false;
    }
    inline static bool Read(skr::archive::JsonReader* r, skr::variant<Ts...>& v)
    {
        SKR_EXPECTED_CHECK(r->StartObject(), false);

        SKR_EXPECTED_CHECK(r->Key(u8"type"), false);
        skr_guid_t index;
        if (!json_read<skr_guid_t>(r, index))
            return false;

        SKR_EXPECTED_CHECK(r->Key(u8"value"), false);
        (void)(((ReadByIndex<Ts>(r, v, index)) != true) && ...);

        SKR_EXPECTED_CHECK(r->EndObject(), false);

        return true;
    }
    inline static bool Write(skr::archive::JsonWriter* json, const skr::variant<Ts...>& v)
    {
        bool success = true;
        skr::visit([&](auto&& value) {
            using raw = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
            if (!json->StartObject().has_value())
            {
                success = false;
                return;
            }
            if (!json->Key(u8"type").has_value())
            {
                success = false;
                return;
            }
            if (!json_write<skr_guid_t>(json, ::skr::rttr::type_id_of<raw>()))
            {
                success = false;
                return;
            }
            if (!json->Key(u8"value").has_value())
            {
                success = false;
                return;
            }
            if (!json_write<decltype(value)>(json, value))
            {
                success = false;
                return;
            }
            if (!json->EndObject().has_value())
            {
                success = false;
                return;
            }
        },
                   v);
        return success;
    }
};
} // namespace skr