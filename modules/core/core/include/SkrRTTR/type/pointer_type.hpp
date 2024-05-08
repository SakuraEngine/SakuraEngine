#pragma once
#include "SkrRTTR/type/generic_type.hpp"

namespace skr::rttr
{
struct SKR_CORE_API PointerType : public GenericType {
    PointerType(Type* target_type, skr::String name);

    inline Type* target_type() const { return _target_type; }

private:
    Type* _target_type;
};
} // namespace skr::rttr