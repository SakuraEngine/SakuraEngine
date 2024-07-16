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
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
namespace skr::json
{
template <class... Ts>
struct ReadTrait<skr::variant<Ts...>> {
    template <class T>
    static bool ReadByIndex(skr::archive::JsonReader* json, skr::variant<Ts...>& value, skr_guid_t index)
    {
        if (index == ::skr::rttr::type_id_of<T>())
        {
            T t;
            if (!skr::json::Read(json, t))
                return false;
            value = std::move(t);
            return true;
        }
        return false;
    }

    static bool Read(skr::archive::JsonReader* json, skr::variant<Ts...>& value)
    {
        json->StartObject();

        json->Key(u8"type");
        skr_guid_t index;
        if (!skr::json::Read<skr_guid_t>(json, index))
            return false;

        json->Key(u8"value");
        (void)(((ReadByIndex<Ts>(json, value, index)) != true) && ...);

        json->EndObject();

        return true;
    }
};
template <class... Ts>
struct WriteTrait<skr::variant<Ts...>> {
    static bool Write(skr::archive::JsonWriter* json, const skr::variant<Ts...>& v)
    {
        skr::visit([&](auto&& value) {
            using raw = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
            json->StartObject();
            json->Key(u8"type");
            skr::json::Write<skr_guid_t>(json, ::skr::rttr::type_id_of<raw>());
            json->Key(u8"value");
            skr::json::Write<decltype(value)>(json, value);
            json->EndObject();
        },
                   v);
        return true;
    }
};
} // namespace skr::json