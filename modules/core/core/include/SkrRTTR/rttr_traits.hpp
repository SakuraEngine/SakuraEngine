#pragma once
#include "SkrRTTR/type_registry.hpp"
#include "SkrContainers/string.hpp"
#include "SkrBase/meta.h"
#include "SkrBase/sinterface.hpp"

// RTTR traits
// 提供非 GenericType 的静态信息：
//  GUID：用于标记此类型，在 TypeSignature 的构建中使用
//  Name：一般为带命名空间的类型名，用于调试、显示等需要展示类型名的地方
//
// RTTRTraits 只提供完全静态的信息，如果需要拿到具体的 Type 对象，需要通过 type_registry 查询，
// 查询结果取决于是否静态注册该类型
namespace skr::rttr
{
template <typename T>
struct RTTRTraits {
    inline static constexpr skr::StringView get_name()
    {
        unimplemented_no_meta(T, "RTTRTraits<T>::get_name() is not implemented");
        return {};
    }
    inline static constexpr GUID get_guid()
    {
        unimplemented_no_meta(T, "RTTRTraits<T>::get_guid() is not implemented");
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
inline Type* type_of()
{
    return get_type_from_guid(type_id_of<T>());
}

} // namespace skr::rttr

//======================================== register marco
#define SKR_RTTR_MAKE_U8(__VALUE) u8##__VALUE
#define SKR_RTTR_TYPE(__TYPE, __GUID)                      \
    namespace skr::rttr                                    \
    {                                                      \
    template <>                                            \
    struct RTTRTraits<__TYPE> {                            \
        inline static constexpr skr::StringView get_name() \
        {                                                  \
            return SKR_RTTR_MAKE_U8(#__TYPE);              \
        }                                                  \
        inline static constexpr GUID get_guid()            \
        {                                                  \
            using namespace skr::guid::literals;           \
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

//======================================== skr types
// TODO. 仅仅为了过编译, 后续实现具体类型
SKR_RTTR_TYPE(skr_guid_t, "80EE37B7-E9C0-40E6-BF2F-51E12053A7A9");
SKR_RTTR_TYPE(skr_md5_t, "F8ABEC14-9436-43B5-A93A-460E7D3CBEC2");
SKR_RTTR_TYPE(skr_rotator_t, "236DFBE6-1554-4BCF-A021-1DAD18BF19B8");
SKR_RTTR_TYPE(skr_float2_t, "CD0FEAC8-C536-4DE7-854A-EC711E59F17D");
SKR_RTTR_TYPE(skr_float3_t, "9E6AB3E9-325F-4224-935C-233CC8DE47B6");
SKR_RTTR_TYPE(skr_float4_t, "38BFB8AD-2287-40FC-8AFE-185CABF84C4A");
SKR_RTTR_TYPE(skr_float4x4_t, "E49D2F13-9DFF-4A7B-8D43-27F68BA932E0");
SKR_RTTR_TYPE(skr_quaternion_t, "51977A88-7095-4FA7-8467-541698803936");
SKR_RTTR_TYPE(::skr::String, "214ED643-54BD-4213-BE37-E336A77FDE84");
SKR_RTTR_TYPE(::skr::StringView, "B799BA81-6009-405D-9131-E4B6101660DC");
SKR_RTTR_TYPE(::skr::SInterface, "244617fe-5274-47bc-aa3d-acd76dbbeddd");
