#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"
#include "SkrRT/containers_new/array.hpp"
#include "SkrRT/containers_new/span.hpp"

namespace skr::rttr
{
struct RUNTIME_API ArrayType : public GenericType {
    ArrayType(Type* target_type, Span<size_t> dimensions);

    bool call_ctor(void* ptr) const override;
    bool call_dtor(void* ptr) const override;
    bool call_copy(void* dst, const void* src) const override;
    bool call_move(void* dst, void* src) const override;
    bool call_assign(void* dst, const void* src) const override;
    bool call_move_assign(void* dst, void* src) const override;
    bool call_hash(const void* ptr, size_t& result) const override;

private:
    Type*         _target_type;
    size_t        _size;
    Array<size_t> _dimensions;
};
} // namespace skr::rttr