#pragma once
#include "SkrRTTR/type/type.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"

namespace skr::rttr
{
template <typename T>
struct PrimitiveType final : public Type {
    PrimitiveType()
        : Type(ETypeCategory::SKR_TYPE_CATEGORY_PRIMITIVE, RTTRTraits<T>::get_name(), RTTRTraits<T>::get_guid(), sizeof(T), alignof(T))
    {
    }
};

template <>
struct PrimitiveType<void> final : public Type {
    PrimitiveType()
        : Type(ETypeCategory::SKR_TYPE_CATEGORY_PRIMITIVE, RTTRTraits<void>::get_name(), RTTRTraits<void>::get_guid(), 1, 1)
    {
    }
};
} // namespace skr::rttr