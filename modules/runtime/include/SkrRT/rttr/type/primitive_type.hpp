#pragma once
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
template <typename T>
struct PrimitiveType final : public Type {
    PrimitiveType()
        : Type(ETypeCategory::SKR_TYPE_CATEGORY_PRIMITIVE, RTTRTraits<T>::get_name(), RTTRTraits<T>::get_guid(), sizeof(T), alignof(T))
    {
    }

    bool call_ctor(void* ptr) const override { return true; }
    bool call_dtor(void* ptr) const override { return true; }
    bool call_copy(void* dst, const void* src) const override
    {
        new (dst) T(*reinterpret_cast<const T*>(src));
        return true;
    }
    bool call_move(void* dst, void* src) const override
    {
        new (dst) T(std::move(*reinterpret_cast<T*>(src)));
        return true;
    }
    bool call_assign(void* dst, const void* src) const override
    {
        (*reinterpret_cast<T*>(dst)) = (*reinterpret_cast<const T*>(src));
        return true;
    }
    bool call_move_assign(void* dst, void* src) const override
    {
        (*reinterpret_cast<T*>(dst)) = std::move(*reinterpret_cast<T*>(src));
        return true;
    }
    bool call_hash(const void* ptr, size_t& result) const override
    {
        result = Hash<T>{}(*reinterpret_cast<const T*>(ptr));
        return true;
    }
};

template <>
struct PrimitiveType<void> final : public Type {
    PrimitiveType()
        : Type(ETypeCategory::SKR_TYPE_CATEGORY_PRIMITIVE, RTTRTraits<void>::get_name(), RTTRTraits<void>::get_guid(), 1, 1)
    {
    }

    bool call_ctor(void* ptr) const override { return true; }
    bool call_dtor(void* ptr) const override { return true; }
    bool call_copy(void* dst, const void* src) const override { return true; }
    bool call_move(void* dst, void* src) const override { return true; }
    bool call_assign(void* dst, const void* src) const override { return true; }
    bool call_move_assign(void* dst, void* src) const override { return true; }
    bool call_hash(const void* ptr, size_t& result) const override { return true; }
};
} // namespace skr::rttr