#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"

namespace skr::rttr
{
// TODO. register pointer type loader in cpp
struct PointerType : public GenericType {

private:
    Type* _target_type;
};
} // namespace skr::rttr