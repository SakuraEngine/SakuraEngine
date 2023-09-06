#pragma once
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/containers_new/umap.hpp"
#include "SkrRT/containers/string.hpp"
#include "SkrRT/containers_new/span.hpp"

namespace skr::rttr
{
struct Field {
    string_view name   = {};
    Type*       type   = nullptr;
    size_t      offset = 0;
};

struct Method {
    using ExecutableType = void (*)(void* self, void* parameters, void* return_value);

    string_view    name            = {};
    Type*          return_type     = nullptr;
    Span<Field>    parameters_type = {};
    ExecutableType executable      = {};
};

struct RecordType : public Type {
    // get methods
    // get fields

private:
    UMap<GUID, Type*> _base_types_map;
};
} // namespace skr::rttr