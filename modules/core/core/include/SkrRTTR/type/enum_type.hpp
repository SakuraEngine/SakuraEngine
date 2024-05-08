#pragma once
#include "SkrContainers/string.hpp"
#include "SkrRTTR/type/type.hpp"
#include "SkrRTTR/enum_value.hpp"

namespace skr::rttr
{
struct SKR_CORE_API EnumType : public Type {
    EnumType(Type* underlying_type, GUID type_id, String name);

    SKR_INLINE Type* underlying_type() const { return _underlying_type; }

    virtual EnumValue value_from_string(StringView str) const       = 0;
    virtual String    value_to_string(const EnumValue& value) const = 0;

private:
    Type* _underlying_type;
};
} // namespace skr::rttr