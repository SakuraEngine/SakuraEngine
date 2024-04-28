#pragma once
#include "SkrRTTR/type/type.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"

namespace skr::rttr
{
template <typename T>
struct PrimitiveType final : public Type {
    PrimitiveType()
        : Type(ETypeCategory::SKR_TYPE_CATEGORY_PRIMITIVE, RTTRTraits<T>::get_name(), RTTRTraits<T>::get_guid(), sizeof(T), alignof(T))
    {
    }

    bool query_feature(ETypeFeature feature) const override
    {
        return true;
    }

    void call_ctor(void* ptr) const override
    {
    }
    void call_dtor(void* ptr) const override
    {
    }
    void call_copy(void* dst, const void* src) const override
    {
        new (dst) T(*reinterpret_cast<const T*>(src));
    }
    void call_move(void* dst, void* src) const override
    {
        new (dst) T(std::move(*reinterpret_cast<T*>(src)));
    }
    void call_assign(void* dst, const void* src) const override
    {
        (*reinterpret_cast<T*>(dst)) = (*reinterpret_cast<const T*>(src));
    }
    void call_move_assign(void* dst, void* src) const override
    {
        (*reinterpret_cast<T*>(dst)) = std::move(*reinterpret_cast<T*>(src));
    }
    size_t call_hash(const void* ptr) const override
    {
        return Hash<T>{}(*reinterpret_cast<const T*>(ptr));
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

    bool query_feature(ETypeFeature feature) const override
    {
        return false;
    }

    void call_ctor(void* ptr) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no ctor method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE()
    }
    void call_dtor(void* ptr) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no dtor method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE()
    }
    void call_copy(void* dst, const void* src) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no copy method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE()
    }
    void call_move(void* dst, void* src) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no move method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE()
    }
    void call_assign(void* dst, const void* src) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no assign method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE()
    }
    void call_move_assign(void* dst, void* src) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no move_assign method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE()
    }
    size_t call_hash(const void* ptr) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no hash method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE();
        return 0;
    }

    int write_binary(const void* dst, skr_binary_writer_t* writer) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no write_binary method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE();
        return 0;
    }
    int read_binary(void* dst, skr_binary_reader_t* reader) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no read_binary method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE();
        return 0;
    }
    void write_json(const void* dst, skr_json_writer_t* writer) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no write_json method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE();
    }
    skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const override
    {
        SKR_LOG_ERROR(u8"[RTTR] void type has no read_json method, before call this function, please check the type feature by query_feature().");
        SKR_UNREACHABLE_CODE();
        return skr::json::error_code::SUCCESS;
    }
};
} // namespace skr::rttr