#pragma once
#include "utils/types.h"
#include "EASTL/string.h"

struct skr_type_t;
struct skr_resource_handle_t;

namespace skr {
namespace type {

template <class T>
struct type_id {
    static const skr_guid_t get()
    {
        static_assert(!sizeof(T), "type_id<T> is not specialized for this type");
    }
};

template <class T>
struct type_of {
    static const skr_type_t* get();
};

#define BASE_TYPE(name, d0, d1, d2, d3_0, d3_1, d3_2, d3_3, d3_4, d3_5, d3_6, d3_7)             \
    template <>                                      \
    struct type_id<name> {                           \
        inline static SKR_CONSTEXPR skr_guid_t get() \
        {                                            \
            return { (d0), (d1), (d2), { (d3_0), (d3_1), (d3_2), (d3_3), (d3_4), (d3_5), (d3_6), (d3_7) } };\
        }                                            \
    };                                               \
    template <>                                      \
    struct type_of<name> {                           \
        RUNTIME_API static const skr_type_t* get();  \
    };

// {d58efbca-9a6c-4f82-b04f-1c057aa924d5}
BASE_TYPE(bool, 0xd58efbca, 0x9a6c, 0x4f82, 0xb0, 0x4f, 0x1c, 0x05, 0x7a, 0xa9, 0x24, 0xd5);
// {7307e684-fc42-4280-92c1-267e32abe57e}
BASE_TYPE(uint32_t, 0x7307e684, 0xfc42, 0x4280, 0x92, 0xc1, 0x26, 0x7e, 0x32, 0xab, 0xe5, 0x7e);
// {a656a839-1c60-422b-80ff-41c99e1b497b}
BASE_TYPE(uint64_t, 0xa656a839, 0x1c60, 0x422b, 0x80, 0xff, 0x41, 0xc9, 0x9e, 0x1b, 0x49, 0x7b);
// {44ee22c3-21bc-44e7-8d10-4946ecd8f80f}
BASE_TYPE(int32_t, 0x44ee22c3, 0x21bc, 0x44e7, 0x8d, 0x10, 0x49, 0x46, 0xec, 0xd8, 0xf8, 0x0f);
// {4b0626b7-a0e0-4ce0-a0b1-8bd9b21d113e}
BASE_TYPE(int64_t, 0x4b0626b7, 0xa0e0, 0x4ce0, 0xa0, 0xb1, 0x8b, 0xd9, 0xb2, 0x1d, 0x11, 0x3e);
// {a4bfe619-9fa9-4d02-8db2-b0ff433bf5e8}
BASE_TYPE(float, 0xa4bfe619, 0x9fa9, 0x4d02, 0x8d, 0xb2, 0xb0, 0xff, 0x43, 0x3b, 0xf5, 0xe8);
// {7b76cf75-3901-4e60-ad6d-d04bbcd86e31}
BASE_TYPE(double, 0x7b76cf75, 0x3901, 0x4e60, 0xad, 0x6d, 0xd0, 0x4b, 0xbc, 0xd8, 0x6e, 0x31);
// {80ee37b7-e9c0-40e6-bf2f-51e12053a7a9}
BASE_TYPE(skr_guid_t, 0x80ee37b7, 0xe9c0, 0x40e6, 0xbf, 0x2f, 0x51, 0xe1, 0x20, 0x53, 0xa7, 0xa9);
// {a9e0ce3d-5e9b-45f1-ac28-b882885c63ab}
BASE_TYPE(skr_resource_handle_t, 0xa9e0ce3d, 0x5e9b, 0x45f1, 0xac, 0x28, 0xb8, 0x82, 0x88, 0x5c, 0x63, 0xab);
// {214ed643-54bd-4213-be37-e336a77fde84}
BASE_TYPE(eastl::string, 0x214ed643, 0x54bd, 0x4213, 0xbe, 0x37, 0xe3, 0x36, 0xa7, 0x7f, 0xde, 0x84);
// {b799ba81-6009-405d-9131-e4b6101660dc}
BASE_TYPE(eastl::string_view, 0xb799ba81, 0x6009, 0x405d, 0x91, 0x31, 0xe4, 0xb6, 0x10, 0x16, 0x60, 0xdc);

#undef BASE_TYPE

} // namespace type
} // namespace skr