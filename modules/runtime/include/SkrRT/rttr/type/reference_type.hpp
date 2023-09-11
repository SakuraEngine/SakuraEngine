#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"

namespace skr::rttr
{
struct SKR_RUNTIME_API ReferenceType : public GenericType {
    ReferenceType(Type* target_type);

    bool call_ctor(void* ptr) const override;
    bool call_dtor(void* ptr) const override;
    bool call_copy(void* dst, const void* src) const override;
    bool call_move(void* dst, void* src) const override;
    bool call_assign(void* dst, const void* src) const override;
    bool call_move_assign(void* dst, void* src) const override;
    bool call_hash(const void* ptr, size_t& result) const override;

    inline Type* target_type() const { return _target_type; }

private:
    Type* _target_type;
};
} // namespace skr::rttr