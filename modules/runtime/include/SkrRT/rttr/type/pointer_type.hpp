#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
struct PointerType : public GenericType {
    PointerType(Type* target_type)
        : _target_type(target_type)
    {
        _generic_guid = kPointerGenericGUID;
    }

    bool call_ctor(void* ptr) const override { return true; }
    bool call_dtor(void* ptr) const override { return true; }
    bool call_copy(void* dst, const void* src) const override
    {
        *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
        return true;
    }
    bool call_move(void* dst, void* src) const override
    {
        *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
        return true;
    }
    bool call_assign(void* dst, const void* src) const override
    {
        *reinterpret_cast<void**>(dst) = *reinterpret_cast<void* const*>(src);
        return true;
    }
    bool call_move_assign(void* dst, void* src) const override
    {
        *reinterpret_cast<void**>(dst) = *reinterpret_cast<void**>(src);
        return true;
    }
    bool call_hash(const void* ptr, size_t& result) const override
    {
        result = reinterpret_cast<size_t>(*reinterpret_cast<void* const*>(ptr));
        return true;
    }

    inline Type* target_type() const { return _target_type; }

private:
    Type* _target_type;
};
} // namespace skr::rttr