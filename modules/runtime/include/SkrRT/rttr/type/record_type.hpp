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
    Type*  _type   = nullptr;
    size_t _offset = 0;
};
template <typename FROM, typename TO>
constexpr inline size_t get_cast_offset()
{
    return reinterpret_cast<size_t>(static_cast<TO*>(reinterpret_cast<FROM*>(0)));
}

// TODO. 直接使用 Field，并传递 Array<Field> 作为参数，使用 move 语义
struct FieldInfo {
    string_view name   = {};
    Type*       type   = nullptr;
    size_t      offset = 0;
};
struct Field {
    string name   = {};
    Type*  type   = nullptr;
    size_t offset = 0;
};

// TODO. 直接使用 Method，并传递 Array<Method> 作为参数，使用 move 语义
struct MethodInfo {
    using ExecutableType = void (*)(void* self, void* parameters, void* return_value);

    string_view    name            = {};
    Type*          return_type     = nullptr;
    Span<Type*>    parameters_type = {};
    ExecutableType executable      = {};
};
struct Method {
    using ExecutableType = void (*)(void* self, void* parameters, void* return_value);

    string         name            = {};
    Type*          return_type     = nullptr;
    Array<Type*>   parameters_type = {};
    ExecutableType executable      = {};
};

struct RecordBasicMethodTable {
    void (*ctor)(void* self)                      = nullptr;
    void (*dtor)(void* self)                      = nullptr;
    void (*copy)(void* dst, const void* src)      = nullptr;
    void (*move)(void* dst, void* src)            = nullptr;
    void (*assign)(void* dst, const void* src)    = nullptr;
    void (*move_assign)(void* dst, void* src)     = nullptr;
    void (*hash)(const void* ptr, size_t& result) = nullptr;
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

    return table;
}

struct SKR_RUNTIME_API RecordType : public Type {
    RecordType(string name, GUID type_id, size_t size, size_t alignment, RecordBasicMethodTable basic_methods, Span<BaseInfo> base_types, Span<FieldInfo> fields, Span<MethodInfo> methods);

    bool call_ctor(void* ptr) const override;
    bool call_dtor(void* ptr) const override;
    bool call_copy(void* dst, const void* src) const override;
    bool call_move(void* dst, void* src) const override;
    bool call_assign(void* dst, const void* src) const override;
    bool call_move_assign(void* dst, void* src) const override;
    bool call_hash(const void* ptr, size_t& result) const override;

    // getter
    const UMap<GUID, BaseInfo>&      base_types() const { return _base_types_map; }
    const MultiUMap<string, Field>&  fields() const { return _fields_map; }
    const MultiUMap<string, Method>& methods() const { return _methods_map; }

    // find methods
    // find fields

private:
    UMap<GUID, BaseInfo>      _base_types_map = {};
    MultiUMap<string, Field>  _fields_map     = {};
    MultiUMap<string, Method> _methods_map    = {};
    RecordBasicMethodTable    _basic_methods  = {};
};
} // namespace skr::rttr