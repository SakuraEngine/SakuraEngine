#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"
#include "SkrRT/containers_new/array.hpp"

namespace skr::rttr
{
// TODO. register array type loader in cpp
struct ArrayType : public GenericType {

private:
    Type*         _target_type;
    size_t        _size;
    Array<size_t> _dimensions;
};
} // namespace skr::rttr