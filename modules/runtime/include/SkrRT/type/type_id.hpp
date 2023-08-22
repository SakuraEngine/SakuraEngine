#pragma once
#include "SkrRT/platform/guid.hpp"
#include "SkrRT/containers/string.hpp"
#include "SkrRT/type/type.h"
#include "SkrRT/type/enum_as_byte.hpp"
#include <string_view> // TODO: replace with skr::string_view

// fwd declares
namespace skr {
namespace type {

struct EnumType;
struct RecordType;
struct ValueSerializePolicy;
using Value = skr_value_t;
using ValueRef = skr_value_ref_t;

SKR_RUNTIME_API skr_type_t* InitializeAndAddToRegistry(skr_guid_t tid, void(*)(RecordType*));
SKR_RUNTIME_API skr_type_t* InitializeAndAddToRegistry(skr_guid_t tid, void(*)(EnumType*));

// TODO: REMOVE THIS
SKR_RUNTIME_API skr_type_t* InitializeAndAddToRegistry(skr_guid_t tid, void(*)());

template <class T>
struct type_register;

template <class T>
struct type_id
{
    inline static constexpr skr_type_t* get() {
        return type_register<T>::get_id();
    }
};

template <class T>
struct type_of
{
    inline static const skr_type_t* get()
    {
        constexpr auto id = type_id<T>::get();
        if (auto type = skr_get_type(&id))
            return type;
        else
            return InitializeAndAddToRegistry(id, &type_register<T>::instantiate_type);
    }
};

#define SKR_RTTI_INLINE_REGISTER_BASE_TYPE(name, s)             \
    template <>                                                 \
    struct type_id<name> {                                      \
        inline static SKR_CONSTEXPR skr_guid_t get()            \
        {                                                       \
            using namespace skr::guid::literals;                \
            return u8##s##_guid;                                \
        }                                                       \
        inline static SKR_CONSTEXPR std::string_view str()      \
        {                                                       \
            return s;                                           \
        }                                                       \
    };                                                          \
    template <>                                                 \
    struct SKR_RUNTIME_API type_register<name> {                \
        static void instantiate_type(RecordType* type);            \
        inline static constexpr skr_guid_t get_id()             \
        {                                                       \
            using namespace skr::guid::literals;                \
            return u8##s##_guid;                                \
        }                                                       \
    };                                                                                                                    

// {873cc4ad-6a4c-4a7f-b59f-57089a3d6cba}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(void*, "873cc4ad-6a4c-4a7f-b59f-57089a3d6cba");
// {244617fe-5274-47bc-aa3d-acd76dbbeddd}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(SInterface, "244617fe-5274-47bc-aa3d-acd76dbbeddd");
// {d58efbca-9a6c-4f82-b04f-1c057aa924d5}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(bool, "D58EFBCA-9A6C-4F82-B04F-1C057AA924D5");
// {7307e684-fc42-4280-92c1-267e32abe57e}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(uint32_t, "7307E684-FC42-4280-92C1-267E32ABE57E");
// {a656a839-1c60-422b-80ff-41c99e1b497b}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(uint64_t, "A656A839-1C60-422B-80FF-41C99E1B497B");
// {44ee22c3-21bc-44e7-8d10-4946ecd8f80f}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(int32_t, "44EE22C3-21BC-44E7-8D10-4946ECD8F80F");
// {4b0626b7-a0e0-4ce0-a0b1-8bd9b21d113e}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(int64_t, "4B0626B7-A0E0-4CE0-A0B1-8BD9B21D113E");
// {a4bfe619-9fa9-4d02-8db2-b0ff433bf5e8}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(float, "A4BFE619-9FA9-4D02-8DB2-B0FF433BF5E8");
// {7b76cf75-3901-4e60-ad6d-d04bbcd86e31}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(double, "7B76CF75-3901-4E60-AD6D-D04BBCD86E31");
// {80ee37b7-e9c0-40e6-bf2f-51e12053a7a9}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_guid_t, "80EE37B7-E9C0-40E6-BF2F-51E12053A7A9");
// {f8abec14-9436-43b5-a93a-460e7d3cbec2}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_md5_t, "F8ABEC14-9436-43B5-A93A-460E7D3CBEC2");
// {a9e0ce3d-5e9b-45f1-ac28-b882885c63ab}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_resource_handle_t, "A9E0CE3D-5E9B-45F1-AC28-B882885C63AB");
// {236dfbe6-1554-4bcf-a021-1dad18bf19b8}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_rotator_t, "236DFBE6-1554-4BCF-A021-1DAD18BF19B8"); 
// {CD0FEAC8-C536-4DE7-854A-EC711E59F17D}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_float2_t, "CD0FEAC8-C536-4DE7-854A-EC711E59F17D");
// {9E6AB3E9-325F-4224-935C-233CC8DE47B6}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_float3_t, "9E6AB3E9-325F-4224-935C-233CC8DE47B6");
// {38BFB8AD-2287-40FC-8AFE-185CABF84C4A}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_float4_t, "38BFB8AD-2287-40FC-8AFE-185CABF84C4A");
// {E49D2F13-9DFF-4A7B-8D43-27F68BA932E0}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_float4x4_t, "E49D2F13-9DFF-4A7B-8D43-27F68BA932E0");
// {51977A88-7095-4FA7-8467-541698803936}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_quaternion_t, "51977A88-7095-4FA7-8467-541698803936");
// {214ed643-54bd-4213-be37-e336a77fde84}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr::string, "214ED643-54BD-4213-BE37-E336A77FDE84");
// {b799ba81-6009-405d-9131-e4b6101660dc}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr::string_view, "B799BA81-6009-405D-9131-E4B6101660DC");

template<class T>
struct type_of<TEnumAsByte<T>>
{
    static const skr_type_t* get()
    {
        return type_of<std::underlying_type_t<T>>::get();
    }
};

} // namespace type
} // namespace skr