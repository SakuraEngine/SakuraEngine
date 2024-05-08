#pragma once
#include "SkrRTTR/type/generic_type.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/span.hpp"

namespace skr::rttr
{
struct SKR_CORE_API VectorType : public GenericType {
    VectorType(Type* target_type, span<size_t> dimensions, skr::String name);

private:
    Type*          _target_type;
    size_t         _size;
    Vector<size_t> _dimensions;
};
} // namespace skr::rttr