#pragma once
#include <SkrBase/containers/string/string_view.hpp>
#include <SkrBase/meta.h>
#include <SkrBase/types.h>

// 提供类型的基本信息
// - get_guid: 代替类名的类型标识符
// - get_name: 类型名称，一般包含完整的命名空间
//
// 对于 Enum 还提供 string 转换接口
// - to_string: 枚举值转字符串
// - from_string: 字符串转枚举值
namespace skr
{
using TypeInfoStringView = skr::container::U8StringView<uint64_t>;

template <typename T>
struct TypeInfo
{
    // inline static constexpr skr::StringView get_name()
    // {
    //     static_assert(std ::is_same_v<T, T*>, "TypeInfo<T>::get_name() is not implemented");
    //     return {};
    // }
    // inline static constexpr GUID get_guid()
    // {
    //     static_assert(std ::is_same_v<T, T*>, "TypeInfo<T>::get_guid() is not implemented");
    //     return {};
    // }
};

template <typename T>
inline constexpr GUID type_id_of()
{
    return TypeInfo<T>::get_guid();
}

template <typename T>
inline constexpr TypeInfoStringView type_name_of()
{
    return TypeInfo<T>::get_name();
}

} // namespace skr

namespace skr::concepts
{
template <typename T>
concept WithRTTRTraits = requires {
    typename TypeInfo<T>;
    { TypeInfo<T>::get_guid() } -> std::convertible_to<GUID>;
    { TypeInfo<T>::get_name() } -> std::convertible_to<TypeInfoStringView>;
};
} // namespace skr::concepts

//======================================== register marco
#define SKR_TYPE_INFO_MAKE_U8(__VALUE) u8##__VALUE
#define SKR_TYPE_INFO(__TYPE, __GUID)                         \
    namespace skr                                             \
    {                                                         \
    template <>                                               \
    struct TypeInfo<__TYPE>                                   \
    {                                                         \
        inline static constexpr TypeInfoStringView get_name() \
        {                                                     \
            return SKR_TYPE_INFO_MAKE_U8(#__TYPE);            \
        }                                                     \
        inline static constexpr GUID get_guid()               \
        {                                                     \
            using namespace skr::literals;                    \
            return u8##__GUID##_guid;                         \
        }                                                     \
    };                                                        \
    }
#define SKR_TYPE_INFO_ENUM(__TYPE, __GUID)                                            \
    namespace skr                                                                     \
    {                                                                                 \
    template <>                                                                       \
    struct TypeInfo<__TYPE>                                                           \
    {                                                                                 \
        inline static constexpr TypeInfoStringView get_name()                         \
        {                                                                             \
            return SKR_TYPE_INFO_MAKE_U8(#__TYPE);                                    \
        }                                                                             \
        inline static constexpr GUID get_guid()                                       \
        {                                                                             \
            using namespace skr::literals;                                            \
            return u8##__GUID##_guid;                                                 \
        }                                                                             \
                                                                                      \
        static TypeInfoStringView to_string(const __TYPE& value);                     \
        static bool               from_string(TypeInfoStringView str, __TYPE& value); \
    };                                                                                \
    }

//======================================== primitive types
SKR_TYPE_INFO(void, "ca27c68d-b987-482c-a031-59112a81eba8")
SKR_TYPE_INFO(bool, "12721970-aa6f-4114-a1d4-e4542dc42956")
SKR_TYPE_INFO(int8_t, "28a92ad9-f90d-443e-b3d2-6cbe7fcb0e3f")
SKR_TYPE_INFO(int16_t, "604d2131-e4e9-4ffc-8fc0-e9aaf5c4012c")
SKR_TYPE_INFO(int32_t, "ed57842f-4aba-44ff-b581-d00a88e031b1")
SKR_TYPE_INFO(int64_t, "6c5df40d-2109-4b2c-b7cc-1e5a37bbf9ed")
SKR_TYPE_INFO(uint8_t, "0d38d18f-7faa-4794-a261-67eadb4e4c13")
SKR_TYPE_INFO(uint16_t, "da5f823f-89d5-4d3a-9ec5-1eeab6a9da0b")
SKR_TYPE_INFO(uint32_t, "582975db-c2a3-4646-bcea-8cc3c1a0f7e5")
SKR_TYPE_INFO(uint64_t, "52b49582-f1f3-4b34-94f2-f89cc40499ca")
SKR_TYPE_INFO(float, "42f9cf37-9995-40a7-9776-1cdb67b98fcf")
SKR_TYPE_INFO(double, "9454d5cd-68dd-4039-8e67-07732de87e5c")

SKR_TYPE_INFO(char, "94e47971-6c88-4216-ae99-71cd9f360992")
SKR_TYPE_INFO(wchar_t, "fe9c2851-7bd5-45b7-8c98-bfabcaaba353")
SKR_TYPE_INFO(char8_t, "089ab395-f8b6-43c3-82ab-01142f3cf34e")
SKR_TYPE_INFO(char16_t, "a4a6ef79-ef3c-4520-a970-4b1163e673c4")
SKR_TYPE_INFO(char32_t, "a9e07135-6091-45dc-b64d-339b42c1bb90")

//======================================== skr types
// GUID & MD5
SKR_TYPE_INFO(skr::MD5, "F8ABEC14-9436-43B5-A93A-460E7D3CBEC2");
SKR_TYPE_INFO(skr::GUID, "80EE37B7-E9C0-40E6-BF2F-51E12053A7A9");
SKR_TYPE_INFO(skr::SHA256, "f2ad2810-7ae4-4721-928f-1fa10a012d9f");