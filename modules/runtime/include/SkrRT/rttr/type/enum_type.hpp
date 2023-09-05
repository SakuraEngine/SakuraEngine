#pragma once
#include "SkrRT/rttr/type/type.hpp"

namespace skr::rttr
{
struct EnumType : public Type {

private:
    Type* _underlying_type;
    // TODO. value <-> string，使用继承的方式来记录？见 EnumToStringTraits
};
} // namespace skr::rttr