#pragma once
// TODO. move to src
#include "SkrRT/rttr/type_loader.hpp"
#include "SkrRT/rttr/type.hpp"
#include "SkrBase/tools/hash.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
template <typename T>
struct PrimitiveType final : public Type {
    PrimitiveType()
    {
        if constexpr (std::is_same_v<T, void>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_VOID;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_BOOL;
        }
        else if constexpr (std::is_same_v<T, int8_t>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_INT8;
        }
        else if constexpr (std::is_same_v<T, int16_t>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_INT16;
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_INT32;
        }
        else if constexpr (std::is_same_v<T, int64_t>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_INT64;
        }
        else if constexpr (std::is_same_v<T, uint8_t>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_UINT8;
        }
        else if constexpr (std::is_same_v<T, uint16_t>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_UINT16;
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_UINT32;
        }
        else if constexpr (std::is_same_v<T, uint64_t>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_UINT64;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_FLOAT;
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            _type_category = ETypeCategory::SKR_TYPE_CATEGORY_DOUBLE;
        }
        _alignment = alignof(T);
        _size      = sizeof(T);
        _type_id   = RTTRTraits<T>::get_guid();
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

template <typename T>
struct PrimitiveTypeLoader final : public TypeLoader {
    Type* load() override
    {
        Type* type = SkrNew<PrimitiveType<T>>();
        return type;
    }
    void destroy(Type* type) override
    {
        SkrDelete(type);
    }
};
} // namespace skr::rttr