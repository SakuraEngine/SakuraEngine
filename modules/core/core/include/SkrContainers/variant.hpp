#pragma once
#include "SkrContainersDef/variant.hpp"

// bin serde
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
namespace skr::binary
{
template <class... Ts>
struct ReadTrait<skr::variant<Ts...>> {
    template <size_t I, class T>
    static bool ReadByIndex(SBinaryReader* archive, skr::variant<Ts...>& value, size_t index)
    {
        if (index == I)
        {
            T t;
            if (!skr::binary::Read(archive, (t))) return false;
            value = std::move(t);
            return true;
        }
        return false;
    }

    template <size_t... Is>
    static bool ReadByIndexHelper(SBinaryReader* archive, skr::variant<Ts...>& value, size_t index, std::index_sequence<Is...>)
    {
        bool result;
        (void)(((result = ReadByIndex<Is, Ts>(archive, value, index)) != 0) && ...);
        return result;
    }

    static bool Read(SBinaryReader* archive, skr::variant<Ts...>& value)
    {
        uint32_t index;
        if (!skr::binary::Read(archive, (index))) return false;
        if (index >= sizeof...(Ts))
            return false;
        return ReadByIndexHelper(archive, value, index, std::make_index_sequence<sizeof...(Ts)>());
    }
};
template <class... Ts>
struct WriteTrait<skr::variant<Ts...>> {
    static int Write(SBinaryWriter* archive, const skr::variant<Ts...>& variant)
    {
        if (!skr::binary::Write(archive, ((uint32_t)variant.index()))) return false;
        bool ret;
        skr::visit([&](auto&& value) {
            ret = skr::binary::Write(archive, value);
        },
                   variant);
        return ret;
    }
};
} // namespace skr::binary

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