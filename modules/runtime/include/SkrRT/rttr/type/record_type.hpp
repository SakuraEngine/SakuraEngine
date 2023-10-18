#pragma once
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/containers_new/multi_umap.hpp"
#include "SkrRT/containers_new/umap.hpp"
#include "SkrRT/containers_new/string.hpp"
#include "SkrRT/containers_new/span.hpp"
#include "SkrRT/containers_new/array.hpp"

namespace skr::rttr
{
struct BaseInfo {
    Type* type                     = nullptr;
    void* (*cast_func)(void* self) = nullptr;
};
struct Field {
    string name   = {};
    Type*  type   = nullptr;
    size_t offset = 0;
};
struct ParameterInfo {
    string name = {};
    Type*  type = nullptr;
};
struct Method {
    using ExecutableType = void (*)(void* self, void* parameters, void* return_value);

    string               name            = {};
    Type*                return_info     = nullptr;
    Array<ParameterInfo> parameters_info = {};
    ExecutableType       executable      = {};
};

struct RecordBasicMethodTable {
    void (*ctor)(void* self)                      = nullptr;
    void (*dtor)(void* self)                      = nullptr;
    void (*copy)(void* dst, const void* src)      = nullptr;
    void (*move)(void* dst, void* src)            = nullptr;
    void (*assign)(void* dst, const void* src)    = nullptr;
    void (*move_assign)(void* dst, void* src)     = nullptr;
    void (*hash)(const void* ptr, size_t& result) = nullptr;

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
        table.ctor = +[](void* self) { new (self) T(); };
    }
    if constexpr (std::is_destructible_v<T>)
    {
        table.dtor = +[](void* self) { ((T*)self)->~T(); };
    }
    if constexpr (std::is_copy_constructible_v<T>)
    {
        table.copy = +[](void* dst, const void* src) { new (dst) T(*(T*)src); };
    }
    if constexpr (std::is_move_constructible_v<T>)
    {
        table.move = +[](void* dst, void* src) { new (dst) T(std::move(*(T*)src)); };
    }
    if constexpr (std::is_copy_assignable_v<T>)
    {
        table.assign = +[](void* dst, const void* src) { *(T*)dst = *(T*)src; };
    }
    if constexpr (std::is_move_assignable_v<T>)
    {
        table.move_assign = +[](void* dst, void* src) { *(T*)dst = std::move(*(T*)src); };
    }
    if constexpr (skr_hashable_v<T>)
    {
        table.hash = +[](const void* ptr, size_t& result) { result = Hash<T>()(*(const T*)ptr); };
    }

    if constexpr (skr::is_complete_serde<skr::binary::WriteTrait<T>>())
    {
        table.write_binary = +[](const void* dst, skr_binary_writer_t* writer) {
            return skr::binary::WriteTrait<T>::Write(writer, *(const T*)dst);
        };
    }
    if constexpr (skr::is_complete_serde<skr::binary::ReadTrait<T>>())
    {
        table.read_binary = +[](void* dst, skr_binary_reader_t* reader) {
            return skr::binary::ReadTrait<T>::Read(reader, *(T*)dst);
        };
    }
    if constexpr (skr::is_complete_serde<skr::json::WriteTrait<T>>())
    {
        table.write_json = +[](const void* dst, skr_json_writer_t* writer) {
            skr::json::WriteTrait<T>::Write(writer, *(const T*)dst);
        };
    }
    if constexpr (skr::is_complete_serde<skr::json::ReadTrait<T>>())
    {
        table.read_json = +[](void* dst, skr::json::value_t&& reader) {
            return skr::json::ReadTrait<T>::Read(std::forward<skr::json::value_t>(reader), *(T*)dst);
        };
    }

    return table;
}

struct SKR_RUNTIME_API RecordType : public Type {
    RecordType(string name, GUID type_id, size_t size, size_t alignment, RecordBasicMethodTable basic_methods);

    bool call_ctor(void* ptr) const override;
    bool call_dtor(void* ptr) const override;
    bool call_copy(void* dst, const void* src) const override;
    bool call_move(void* dst, void* src) const override;
    bool call_assign(void* dst, const void* src) const override;
    bool call_move_assign(void* dst, void* src) const override;
    bool call_hash(const void* ptr, size_t& result) const override;

    int                   write_binary(const void* dst, skr_binary_writer_t* writer) const override;
    int                   read_binary(void* dst, skr_binary_reader_t* reader) const override;
    void                  write_json(const void* dst, skr_json_writer_t* writer) const override;
    skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const override;

    // setup
    SKR_INLINE void set_base_types(UMap<GUID, BaseInfo> base_types) { _base_types_map = std::move(base_types); }
    SKR_INLINE void set_fields(MultiUMap<string, Field> fields) { _fields_map = std::move(fields); }
    SKR_INLINE void set_methods(MultiUMap<string, Method> methods) { _methods_map = std::move(methods); }

    // getter
    SKR_INLINE const UMap<GUID, BaseInfo>& base_types() const { return _base_types_map; }
    SKR_INLINE const MultiUMap<string, Field>& fields() const { return _fields_map; }
    SKR_INLINE const MultiUMap<string, Method>& methods() const { return _methods_map; }

    // find base
    void* cast_to(const Type* target_type, void* p_self) const;

    // find methods
    // find fields

private:
    UMap<GUID, BaseInfo>      _base_types_map = {};
    MultiUMap<string, Field>  _fields_map     = {};
    MultiUMap<string, Method> _methods_map    = {};
    RecordBasicMethodTable    _basic_methods  = {};
};
} // namespace skr::rttr