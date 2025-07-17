#pragma once
#include "SkrRTTR/type_registry.hpp"
#include "SkrContainersDef/string.hpp"
#include "SkrBase/meta.h"
#include "SkrBase/math.h"

// RTTR traits
// 提供非 GenericType 的静态信息：
//  GUID：用于标记此类型，在 TypeSignature 的构建中使用
//  Name：一般为带命名空间的类型名，用于调试、显示等需要展示类型名的地方
//
// RTTRTraits 只提供完全静态的信息，如果需要拿到具体的 Type 对象，需要通过 type_registry 查询，
// 查询结果取决于是否静态注册该类型
namespace skr
{
template <typename T>
struct RTTRTraits {
    inline static constexpr skr::StringView get_name()
    {
        static_assert(std ::is_same_v<T, T*>, "RTTRTraits<T>::get_name() is not implemented");
        return {};
    }
    inline static constexpr GUID get_guid()
    {
        static_assert(std ::is_same_v<T, T*>, "RTTRTraits<T>::get_guid() is not implemented");
        return {};
    }
};

template <typename T>
inline constexpr GUID type_id_of()
{
    return RTTRTraits<T>::get_guid();
}

template <typename T>
inline constexpr skr::StringView type_name_of()
{
    return RTTRTraits<T>::get_name();
}

template <typename T>
inline RTTRType* type_of()
{
    return get_type_from_guid(type_id_of<T>());
}

} // namespace skr

//======================================== register marco
#define SKR_RTTR_MAKE_U8(__VALUE) u8##__VALUE
#define SKR_RTTR_TYPE(__TYPE, __GUID)                      \
    namespace skr                                          \
    {                                                      \
    template <>                                            \
    struct RTTRTraits<__TYPE> {                            \
        inline static constexpr skr::StringView get_name() \
        {                                                  \
            return SKR_RTTR_MAKE_U8(#__TYPE);              \
        }                                                  \
        inline static constexpr GUID get_guid()            \
        {                                                  \
            using namespace skr::literals;                 \
            return u8##__GUID##_guid;                      \
        }                                                  \
    };                                                     \
    }

//======================================== primitive types
SKR_RTTR_TYPE(void, "ca27c68d-b987-482c-a031-59112a81eba8")
SKR_RTTR_TYPE(bool, "12721970-aa6f-4114-a1d4-e4542dc42956")
SKR_RTTR_TYPE(int8_t, "28a92ad9-f90d-443e-b3d2-6cbe7fcb0e3f")
SKR_RTTR_TYPE(int16_t, "604d2131-e4e9-4ffc-8fc0-e9aaf5c4012c")
SKR_RTTR_TYPE(int32_t, "ed57842f-4aba-44ff-b581-d00a88e031b1")
SKR_RTTR_TYPE(int64_t, "6c5df40d-2109-4b2c-b7cc-1e5a37bbf9ed")
SKR_RTTR_TYPE(uint8_t, "0d38d18f-7faa-4794-a261-67eadb4e4c13")
SKR_RTTR_TYPE(uint16_t, "da5f823f-89d5-4d3a-9ec5-1eeab6a9da0b")
SKR_RTTR_TYPE(uint32_t, "582975db-c2a3-4646-bcea-8cc3c1a0f7e5")
SKR_RTTR_TYPE(uint64_t, "52b49582-f1f3-4b34-94f2-f89cc40499ca")
SKR_RTTR_TYPE(float, "42f9cf37-9995-40a7-9776-1cdb67b98fcf")
SKR_RTTR_TYPE(double, "9454d5cd-68dd-4039-8e67-07732de87e5c")

SKR_RTTR_TYPE(char, "94e47971-6c88-4216-ae99-71cd9f360992")
SKR_RTTR_TYPE(wchar_t, "fe9c2851-7bd5-45b7-8c98-bfabcaaba353")
SKR_RTTR_TYPE(char8_t, "089ab395-f8b6-43c3-82ab-01142f3cf34e")
SKR_RTTR_TYPE(char16_t, "a4a6ef79-ef3c-4520-a970-4b1163e673c4")
SKR_RTTR_TYPE(char32_t, "a9e07135-6091-45dc-b64d-339b42c1bb90")

//======================================== skr types
// GUID & MD5
SKR_RTTR_TYPE(skr::MD5, "F8ABEC14-9436-43B5-A93A-460E7D3CBEC2");
SKR_RTTR_TYPE(skr::GUID, "80EE37B7-E9C0-40E6-BF2F-51E12053A7A9");

// string types
SKR_RTTR_TYPE(skr::String, "214ED643-54BD-4213-BE37-E336A77FDE84");
SKR_RTTR_TYPE(skr::StringView, "B799BA81-6009-405D-9131-E4B6101660DC");

// math misc types
SKR_RTTR_TYPE(skr::RotatorF, "236DFBE6-1554-4BCF-A021-1DAD18BF19B8");
SKR_RTTR_TYPE(skr::RotatorD, "e79cb48b-e8eb-496b-a021-e1800ba2471d");
SKR_RTTR_TYPE(skr::QuatF, "44d46c00-7acd-4b58-8b04-f2f8227013df");
SKR_RTTR_TYPE(skr::QuatD, "57c215a0-ac90-4340-a20b-8b41b45b4b7a");
SKR_RTTR_TYPE(skr::TransformF, "e9018d16-bc41-4772-b53d-110a2c3d658d");
SKR_RTTR_TYPE(skr::TransformD, "15f786c7-d075-4e8d-82bd-6ae43032f9c1");

// math vector types
SKR_RTTR_TYPE(skr::float2, "CD0FEAC8-C536-4DE7-854A-EC711E59F17D");
SKR_RTTR_TYPE(skr::float3, "50ef1762-78f0-48ce-b4f8-373d045cba8c");
SKR_RTTR_TYPE(skr::float4, "36250d48-1a73-4db4-a6fb-790e368f4dfb");
SKR_RTTR_TYPE(skr::double2, "ed9e1f47-ef21-47cf-a590-89f2d909556f");
SKR_RTTR_TYPE(skr::double3, "58e7532a-f113-4f2a-834c-eb70ebef6ad8");
SKR_RTTR_TYPE(skr::double4, "7c1af989-5102-4e2a-a1ec-52cd53834080");
SKR_RTTR_TYPE(skr::int2, "d85cee52-dc50-416f-9d6f-4b336e940483");
SKR_RTTR_TYPE(skr::int3, "d7c9b298-60b6-4bac-902f-f100fcd3022c");
SKR_RTTR_TYPE(skr::int4, "1fb93344-6acc-4077-8027-8eb96295f7b2");
SKR_RTTR_TYPE(skr::uint2, "de9d63e1-c83f-4655-a1f7-3954fcb9e0be");
SKR_RTTR_TYPE(skr::uint3, "05e66702-a75a-4249-a107-c37b45f07a46");
SKR_RTTR_TYPE(skr::uint4, "16f0850f-0353-4a5e-8ce3-464d2ae83e37");
SKR_RTTR_TYPE(skr::long2, "73ace8f7-e326-497c-bd2b-502c6b9bb055");
SKR_RTTR_TYPE(skr::long3, "198e20b2-1b77-4d9b-a49b-b852f65f296a");
SKR_RTTR_TYPE(skr::long4, "e821537d-e015-4e63-bb43-7245eb1745ab");
SKR_RTTR_TYPE(skr::ulong2, "93c4ee8a-b8a3-4083-9e7a-e37a9280a1fd");
SKR_RTTR_TYPE(skr::ulong3, "d5058f07-0a1b-40c7-b445-cef239d1ae81");
SKR_RTTR_TYPE(skr::ulong4, "9eea4680-0c5c-40ce-86a1-879ff3b7b50c");
SKR_RTTR_TYPE(skr::bool2, "dfa2c74c-5a6b-4659-aa68-be5fbe9b0a61");
SKR_RTTR_TYPE(skr::bool3, "4ad5c5ae-9f62-4d34-83dc-316e49cec451");
SKR_RTTR_TYPE(skr::bool4, "845ae22a-6198-4bcb-90ed-acc68df06def");

// matrix types
SKR_RTTR_TYPE(skr::float3x3, "391ae9db-ba6c-4bb5-9ce1-67363b9cd437");
SKR_RTTR_TYPE(skr::float4x4, "b8b4c30c-96bc-491a-bb00-9d09a13f0405");
SKR_RTTR_TYPE(skr::double3x3, "04854107-ff25-4a46-9dff-9de7fa363943");
SKR_RTTR_TYPE(skr::double4x4, "96422197-3e05-4622-bb80-471019a798e9");
