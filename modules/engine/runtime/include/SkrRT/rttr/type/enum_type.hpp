#pragma once
#include "SkrContainers/string.hpp"
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/rttr/enum_value.hpp"

namespace skr::rttr
{
struct SKR_RUNTIME_API EnumType : public Type {
    EnumType(Type* underlying_type, GUID type_id, String name);

    SKR_INLINE Type* underlying_type() const { return _underlying_type; }

    void   call_ctor(void* ptr) const override;
    void   call_dtor(void* ptr) const override;
    void   call_copy(void* dst, const void* src) const override;
    void   call_move(void* dst, void* src) const override;
    void   call_assign(void* dst, const void* src) const override;
    void   call_move_assign(void* dst, void* src) const override;
    size_t call_hash(const void* ptr) const override;

    virtual EnumValue value_from_string(StringView str) const       = 0;
    virtual String    value_to_string(const EnumValue& value) const = 0;

private:
    Type* _underlying_type;
};
} // namespace skr::rttr