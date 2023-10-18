#pragma once
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
#include "SkrRT/serde/json/reader.h"
#include "SkrRT/serde/json/writer.h"
#include "SkrRT/serde/binary/reader.h"
#include "SkrRT/serde/binary/writer.h"

namespace skr::rttr
{
template <typename T>
struct PrimitiveType final : public Type {
    PrimitiveType()
        : Type(ETypeCategory::SKR_TYPE_CATEGORY_PRIMITIVE, RTTRTraits<T>::get_name(), RTTRTraits<T>::get_guid(), sizeof(T), alignof(T))
    {
    }

    bool call_ctor(void* ptr) const override { return true; }
    bool call_dtor(void* ptr) const override { return true; }
    bool call_copy(void* dst, const void* src) const override
    {
        new (dst) T(*reinterpret_cast<const T*>(src));
        return true;
    }
    bool call_move(void* dst, void* src) const override
    {
        new (dst) T(std::move(*reinterpret_cast<T*>(src)));
        return true;
    }
    bool call_assign(void* dst, const void* src) const override
    {
        (*reinterpret_cast<T*>(dst)) = (*reinterpret_cast<const T*>(src));
        return true;
    }
    bool call_move_assign(void* dst, void* src) const override
    {
        (*reinterpret_cast<T*>(dst)) = std::move(*reinterpret_cast<T*>(src));
        return true;
    }
    bool call_hash(const void* ptr, size_t& result) const override
    {
        result = Hash<T>{}(*reinterpret_cast<const T*>(ptr));
        return true;
    }

    int write_binary(const void* dst, skr_binary_writer_t* writer) const override
    {
        return skr::binary::WriteTrait<T>::Write(writer, *reinterpret_cast<const T*>(dst));
    }
    int read_binary(void* dst, skr_binary_reader_t* reader) const override
    {
        return skr::binary::ReadTrait<T>::Read(reader, *reinterpret_cast<T*>(dst));
    }
    void write_json(const void* dst, skr_json_writer_t* writer) const override
    {
        skr::json::WriteTrait<T>::Write(writer, *reinterpret_cast<const T*>(dst));
    }
    skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const override
    {
        return skr::json::ReadTrait<T>::Read(std::forward<skr::json::value_t>(reader), *reinterpret_cast<T*>(dst));
    }
};

template <>
struct PrimitiveType<void> final : public Type {
    PrimitiveType()
        : Type(ETypeCategory::SKR_TYPE_CATEGORY_PRIMITIVE, RTTRTraits<void>::get_name(), RTTRTraits<void>::get_guid(), 1, 1)
    {
    }

    bool call_ctor(void* ptr) const override { return true; }
    bool call_dtor(void* ptr) const override { return true; }
    bool call_copy(void* dst, const void* src) const override { return true; }
    bool call_move(void* dst, void* src) const override { return true; }
    bool call_assign(void* dst, const void* src) const override { return true; }
    bool call_move_assign(void* dst, void* src) const override { return true; }
    bool call_hash(const void* ptr, size_t& result) const override { return true; }

    int write_binary(const void* dst, skr_binary_writer_t* writer) const override
    {
        SKR_UNREACHABLE_CODE();
        return 0;
    }
    int read_binary(void* dst, skr_binary_reader_t* reader) const override
    {
        SKR_UNREACHABLE_CODE();
        return 0;
    }
    void write_json(const void* dst, skr_json_writer_t* writer) const override
    {
        SKR_UNREACHABLE_CODE();
    }
    skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const override
    {
        SKR_UNREACHABLE_CODE();
        return skr::json::error_code::SUCCESS;
    }
};
} // namespace skr::rttr