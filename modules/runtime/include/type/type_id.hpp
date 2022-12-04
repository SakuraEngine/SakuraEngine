#pragma once
#include "platform/configure.h"
#include "utils/types.h"
#include "enum_as_byte.hpp"
#include <string_view>

// fwd declares
typedef struct skr_type_t skr_type_t;
typedef struct skr_value_t skr_value_t;
typedef struct skr_value_ref_t skr_value_ref_t;
typedef struct skr_field_t skr_field_t;
typedef struct skr_method_t skr_method_t;
typedef skr_guid_t skr_type_id_t;
struct skr_resource_handle_t;
struct skr_binary_writer_t;
struct skr_binary_reader_t;
namespace skr
{
namespace type
{
struct ValueSerializePolicy;
using Value = skr_value_t;
using ValueRef = skr_value_ref_t;
} // namespace type
} // namespace skr

namespace skr {
namespace type {

template <class T>
struct type_id;

template <class T>
struct type_of;

#define SKR_RTTI_INLINE_REGISTER_BASE_TYPE(name, d0, d1, d2, d3_0, d3_1, d3_2, d3_3, d3_4, d3_5, d3_6, d3_7, s)             \
    template <>                                      \
    struct type_id<name> {                           \
        inline static SKR_CONSTEXPR skr_guid_t get() \
        {                                            \
            return { (d0), (d1), (d2), { (d3_0), (d3_1), (d3_2), (d3_3), (d3_4), (d3_5), (d3_6), (d3_7) } };\
        }                                            \
        inline static SKR_CONSTEXPR std::string_view str()\
        {                                            \
            return s;                                \
        }                                            \
    };                                               \
    template <>                                      \
    struct type_of<name> {                           \
        RUNTIME_API static const skr_type_t* get();  \
    };

// {d58efbca-9a6c-4f82-b04f-1c057aa924d5}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(bool, 0xd58efbca, 0x9a6c, 0x4f82, 0xb0, 0x4f, 0x1c, 0x05, 0x7a, 0xa9, 0x24, 0xd5, "D58EFBCA-9A6C-4F82-B04F-1C057AA924D5");
// {7307e684-fc42-4280-92c1-267e32abe57e}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(uint32_t, 0x7307e684, 0xfc42, 0x4280, 0x92, 0xc1, 0x26, 0x7e, 0x32, 0xab, 0xe5, 0x7e, "7307E684-FC42-4280-92C1-267E32ABE57E");
// {a656a839-1c60-422b-80ff-41c99e1b497b}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(uint64_t, 0xa656a839, 0x1c60, 0x422b, 0x80, 0xff, 0x41, 0xc9, 0x9e, 0x1b, 0x49, 0x7b, "A656A839-1C60-422B-80FF-41C99E1B497B");
// {44ee22c3-21bc-44e7-8d10-4946ecd8f80f}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(int32_t, 0x44ee22c3, 0x21bc, 0x44e7, 0x8d, 0x10, 0x49, 0x46, 0xec, 0xd8, 0xf8, 0x0f, "44EE22C3-21BC-44E7-8D10-4946ECD8F80F");
// {4b0626b7-a0e0-4ce0-a0b1-8bd9b21d113e}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(int64_t, 0x4b0626b7, 0xa0e0, 0x4ce0, 0xa0, 0xb1, 0x8b, 0xd9, 0xb2, 0x1d, 0x11, 0x3e, "4B0626B7-A0E0-4CE0-A0B1-8BD9B21D113E");
// {a4bfe619-9fa9-4d02-8db2-b0ff433bf5e8}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(float, 0xa4bfe619, 0x9fa9, 0x4d02, 0x8d, 0xb2, 0xb0, 0xff, 0x43, 0x3b, 0xf5, 0xe8, "A4BFE619-9FA9-4D02-8DB2-B0FF433BF5E8");
// {7b76cf75-3901-4e60-ad6d-d04bbcd86e31}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(double, 0x7b76cf75, 0x3901, 0x4e60, 0xad, 0x6d, 0xd0, 0x4b, 0xbc, 0xd8, 0x6e, 0x31, "7B76CF75-3901-4E60-AD6D-D04BBCD86E31");
// {80ee37b7-e9c0-40e6-bf2f-51e12053a7a9}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_guid_t, 0x80ee37b7, 0xe9c0, 0x40e6, 0xbf, 0x2f, 0x51, 0xe1, 0x20, 0x53, 0xa7, 0xa9, "80EE37B7-E9C0-40E6-BF2F-51E12053A7A9");
// {f8abec14-9436-43b5-a93a-460e7d3cbec2}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_md5_t, 0xf8abec14, 0x9436, 0x43b5, 0xa9, 0x3a, 0x46, 0x0e, 0x7d, 0x3c, 0xbe, 0xc2, "F8ABEC14-9436-43B5-A93A-460E7D3CBEC2");
// {a9e0ce3d-5e9b-45f1-ac28-b882885c63ab}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_resource_handle_t, 0xa9e0ce3d, 0x5e9b, 0x45f1, 0xac, 0x28, 0xb8, 0x82, 0x88, 0x5c, 0x63, 0xab, "A9E0CE3D-5E9B-45F1-AC28-B882885C63AB");
// {236dfbe6-1554-4bcf-a021-1dad18bf19b8}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_rotator_t, 0x236dfbe6, 0x1554, 0x4bcf, 0xa0, 0x21, 0x1d, 0xad, 0x18, 0xbf, 0x19, 0xb8, "236DFBE6-1554-4BCF-A021-1DAD18BF19B8"); 
// {CD0FEAC8-C536-4DE7-854A-EC711E59F17D}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_float2_t, 0xcd0feac8, 0xc536, 0x4de7, 0x85, 0x4a, 0xec, 0x71, 0x1e, 0x59, 0xf1, 0x7d, "CD0FEAC8-C536-4DE7-854A-EC711E59F17D");
// {9E6AB3E9-325F-4224-935C-233CC8DE47B6}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_float3_t, 0x9e6ab3e9, 0x325f, 0x4224, 0x93, 0x5c, 0x23, 0x3c, 0xc8, 0xde, 0x47, 0xb6, "9E6AB3E9-325F-4224-935C-233CC8DE47B6");
// {38BFB8AD-2287-40FC-8AFE-185CABF84C4A}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_float4_t, 0x38bfb8ad, 0x2287, 0x40fc, 0x8a, 0xfe, 0x18, 0x5c, 0xab, 0xf8, 0x4c, 0x4a, "38BFB8AD-2287-40FC-8AFE-185CABF84C4A");
// {E49D2F13-9DFF-4A7B-8D43-27F68BA932E0}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_float4x4_t, 0xe49d2f13, 0x9dff, 0x4a7b, 0x8d, 0x43, 0x27, 0xf6, 0x8b, 0xa9, 0x32, 0xe0, "E49D2F13-9DFF-4A7B-8D43-27F68BA932E0");
// {51977A88-7095-4FA7-8467-541698803936}
SKR_RTTI_INLINE_REGISTER_BASE_TYPE(skr_quaternion_t, 0x51977a88, 0x7095, 0x4fa7, 0x84, 0x67, 0x54, 0x16, 0x98, 0x80, 0x39, 0x36, "51977A88-7095-4FA7-8467-541698803936");


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