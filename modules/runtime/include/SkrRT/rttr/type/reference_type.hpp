#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"

namespace skr::rttr
{
// TODO. register reference type loader in cpp
struct ReferenceType : public GenericType {

private:
    Type* _target_type;
};
} // namespace skr::rttr