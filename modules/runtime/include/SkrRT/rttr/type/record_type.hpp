#pragma once
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/containers_new/multi_umap.hpp"
#include "SkrRT/containers_new/umap.hpp"
#include "SkrRT/containers_new/string.hpp"
#include "SkrRT/containers_new/span.hpp"
#include "SkrRT/containers_new/array.hpp"

namespace skr::rttr
{
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

struct SKR_RUNTIME_API RecordType : public Type {
    RecordType(GUID type_id, size_t size, size_t alignment, Span<Type*> base_types, Span<FieldInfo> fields, Span<MethodInfo> methods);

    // find methods
    // find fields

private:
    UMap<GUID, Type*>         _base_types_map = {};
    MultiUMap<string, Field>  _fields         = {};
    MultiUMap<string, Method> _methods        = {};
};
} // namespace skr::rttr