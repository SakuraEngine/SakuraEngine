#pragma once
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/containers_new/multi_umap.hpp"
#include "SkrRT/containers_new/umap.hpp"
#include "SkrRT/containers_new/string.hpp"
#include "SkrRT/containers_new/span.hpp"
#include "SkrRT/containers_new/vector.hpp"

namespace skr::rttr
{
namespace helper
{
namespace detail
{
template <typename T>
using has_binary_write = decltype(skr::binary::WriteTrait<T>::Write(std::declval<skr_binary_writer_t*>(), std::declval<const T&>()));
template <typename T>
using has_binary_read = decltype(skr::binary::ReadTrait<T>::Read(std::declval<skr_binary_reader_t*>(), std::declval<T&>()));
template <typename T>
using has_json_write = decltype(skr::json::WriteTrait<T>::Write(std::declval<skr_json_writer_t*>(), std::declval<const T&>()));
template <typename T>
using has_json_read = decltype(skr::json::ReadTrait<T>::Read(std::declval<skr::json::value_t&&>(), std::declval<T&>()));
} // namespace detail
template <typename T>
static constexpr bool has_binary_write_v = skr::is_detected_v<detail::has_binary_write, T>;
template <typename T>
static constexpr bool has_binary_read_v = skr::is_detected_v<detail::has_binary_read, T>;
template <typename T>
static constexpr bool has_json_write_v = skr::is_detected_v<detail::has_json_write, T>;
template <typename T>
static constexpr bool has_json_read_v = skr::is_detected_v<detail::has_json_read, T>;
} // namespace helper

struct BaseInfo {
    Type* type                     = nullptr;
    void* (*cast_func)(void* self) = nullptr;
};
struct Field {
    skr::String name   = {};
    Type*  type   = nullptr;
    size_t offset = 0;
};
struct ParameterInfo {
    skr::String name = {};
    Type*  type = nullptr;
};
struct Method {
    using ExecutableType = void (*)(void* self, void* parameters, void* return_value);

    skr::String               name            = {};
    Type*                return_info     = nullptr;
    Vector<ParameterInfo> parameters_info = {};
    ExecutableType       executable      = {};
};

struct RecordBasicMethodTable {
    void   (*ctor)(void* self)                   = nullptr;
    void   (*dtor)(void* self)                   = nullptr;
    void   (*copy)(void* dst, const void* src)   = nullptr;
    void   (*move)(void* dst, void* src)         = nullptr;
    void   (*assign)(void* dst, const void* src) = nullptr;
    void   (*move_assign)(void* dst, void* src)  = nullptr;
    size_t (*hash)(const void* ptr)              = nullptr;

    int                   (*write_binary)(const void* dst, skr_binary_writer_t* writer) = nullptr;
    int                   (*read_binary)(void* dst, skr_binary_reader_t* reader)        = nullptr;
    void                  (*write_json)(const void* dst, skr_json_writer_t* writer)     = nullptr;
    skr::json::error_code (*read_json)(void* dst, skr::json::value_t&& reader)          = nullptr;
};
template <typename T>
SKR_INLINE RecordBasicMethodTable make_record_basic_method_table()
{
    RecordBasicMethodTable table = {};
    if constexpr (std::is_default_constructible_v<T>)
    {
        table.ctor = +[](void* self) {
            new (self) T();
        };
    }
    if constexpr (std::is_destructible_v<T>)
    {
        table.dtor = +[](void* self) {
            ((T*)self)->~T();
        };
    }
    if constexpr (std::is_copy_constructible_v<T>)
    {
        table.copy = +[](void* dst, const void* src) {
            new (dst) T(*(T*)src);
        };
    }
    if constexpr (std::is_move_constructible_v<T>)
    {
        table.move = +[](void* dst, void* src) {
            new (dst) T(std::move(*(T*)src));
        };
    }
    if constexpr (std::is_copy_assignable_v<T>)
    {
        table.assign = +[](void* dst, const void* src) {
            *(T*)dst = *(T*)src;
        };
    }
    if constexpr (std::is_move_assignable_v<T>)
    {
        table.move_assign = +[](void* dst, void* src) {
            *(T*)dst = std::move(*(T*)src);
        };
    }
    if constexpr (skr_hashable_v<T>)
    {
        table.hash = +[](const void* ptr) {
            return Hash<T>()(*(const T*)ptr);
        };
    }

    if constexpr (helper::has_binary_write_v<T>)
    {
        table.write_binary = +[](const void* dst, skr_binary_writer_t* writer) {
            return skr::binary::WriteTrait<T>::Write(writer, *(const T*)dst);
        };
    }
    if constexpr (helper::has_binary_write_v<T>)
    {
        table.read_binary = +[](void* dst, skr_binary_reader_t* reader) {
            return skr::binary::ReadTrait<T>::Read(reader, *(T*)dst);
        };
    }
    if constexpr (helper::has_json_write_v<T>)
    {
        table.write_json = +[](const void* dst, skr_json_writer_t* writer) {
            skr::json::WriteTrait<T>::Write(writer, *(const T*)dst);
        };
    }
    if constexpr (helper::has_json_read_v<T>)
    {
        table.read_json = +[](void* dst, skr::json::value_t&& reader) {
            return skr::json::ReadTrait<T>::Read(std::forward<skr::json::value_t>(reader), *(T*)dst);
        };
    }

    return table;
}

struct SKR_RUNTIME_API RecordType : public Type {
    RecordType(skr::String name, GUID type_id, size_t size, size_t alignment, RecordBasicMethodTable basic_methods);

    bool query_feature(ETypeFeature feature) const override;

    void   call_ctor(void* ptr) const override;
    void   call_dtor(void* ptr) const override;
    void   call_copy(void* dst, const void* src) const override;
    void   call_move(void* dst, void* src) const override;
    void   call_assign(void* dst, const void* src) const override;
    void   call_move_assign(void* dst, void* src) const override;
    size_t call_hash(const void* ptr) const override;

    int                   write_binary(const void* dst, skr_binary_writer_t* writer) const override;
    int                   read_binary(void* dst, skr_binary_reader_t* reader) const override;
    void                  write_json(const void* dst, skr_json_writer_t* writer) const override;
    skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const override;

    // setup
    void set_base_types(UMap<GUID, BaseInfo> base_types);
    void set_fields(MultiUMap<skr::String, Field> fields);
    void set_methods(MultiUMap<skr::String, Method> methods);

    // getter
    SKR_INLINE const UMap<GUID, BaseInfo>& base_types() const { return _base_types_map; }
    SKR_INLINE const MultiUMap<skr::String, Field>& fields() const { return _fields_map; }
    SKR_INLINE const MultiUMap<skr::String, Method>& methods() const { return _methods_map; }

    // find base
    void* cast_to(const Type* target_type, void* p_self) const;

    // find methods
    // find fields

private:
    UMap<GUID, BaseInfo>      _base_types_map = {};
    MultiUMap<skr::String, Field>  _fields_map     = {};
    MultiUMap<skr::String, Method> _methods_map    = {};
    RecordBasicMethodTable    _basic_methods  = {};
};
} // namespace skr::rttr