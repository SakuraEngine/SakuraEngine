#pragma once
#include "SkrRT/platform/configure.h"
#include "SkrRT/rttr/guid.hpp"
#include "SkrRT/rttr/type_desc.hpp"
#include "SkrRT/rttr/type_registry.hpp"

// RTTR traits
// 提供部分静态类型功能，从动态角度来说，实际上只是一层皮
namespace skr::rttr
{
struct Type;

template <typename T>
struct RTTRTraits {
    static GUID           get_guid();
    static Span<TypeDesc> get_type_desc();
    static Type*          get_type();
};

template <typename T>
SKR_INLINE GUID type_id() SKR_NOEXCEPT
{
    return RTTRTraits<T>::get_guid();
}
template <typename T>
SKR_INLINE Span<TypeDesc> type_desc() SKR_NOEXCEPT
{
    return RTTRTraits<T>::get_type_desc();
}
template <typename T>
SKR_INLINE Type* type_of() SKR_NOEXCEPT
{
    return RTTRTraits<T>::get_type();
}
} // namespace skr::rttr

// primitive types
namespace skr::rttr
{
#define SKR_RTTR_PRIMITIVE_TYPE_TRAITS(__TYPE, __GUID)             \
    template <>                                                    \
    struct RTTRTraits<__TYPE> {                                    \
        static GUID           get_guid() { return __GUID##_guid; } \
        static Span<TypeDesc> get_type_desc()                      \
        {                                                          \
            static TypeDesc desc = { get_guid() };                 \
            return { &desc, 1 };                                   \
        }                                                          \
        static Type* get_type()                                    \
        {                                                          \
            static Type* type = nullptr;                           \
            if (!type)                                             \
            {                                                      \
                type = get_type_from_guid(get_guid());             \
            }                                                      \
            return type;                                           \
        }                                                          \
    };

SKR_RTTR_PRIMITIVE_TYPE_TRAITS(void, "ca27c68d-b987-482c-a031-59112a81eba8")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(bool, "12721970-aa6f-4114-a1d4-e4542dc42956")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(int8_t, "28a92ad9-f90d-443e-b3d2-6cbe7fcb0e3f")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(int16_t, "604d2131-e4e9-4ffc-8fc0-e9aaf5c4012c")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(int32_t, "ed57842f-4aba-44ff-b581-d00a88e031b1")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(int64_t, "6c5df40d-2109-4b2c-b7cc-1e5a37bbf9ed")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(uint8_t, "0d38d18f-7faa-4794-a261-67eadb4e4c13")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(uint16_t, "da5f823f-89d5-4d3a-9ec5-1eeab6a9da0b")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(uint32_t, "582975db-c2a3-4646-bcea-8cc3c1a0f7e5")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(uint64_t, "52b49582-f1f3-4b34-94f2-f89cc40499ca")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(float, "42f9cf37-9995-40a7-9776-1cdb67b98fcf")
SKR_RTTR_PRIMITIVE_TYPE_TRAITS(double, "9454d5cd-68dd-4039-8e67-07732de87e5c")

#undef SKR_RTTR_PRIMITIVE_TYPE_TRAITS
} // namespace skr::rttr