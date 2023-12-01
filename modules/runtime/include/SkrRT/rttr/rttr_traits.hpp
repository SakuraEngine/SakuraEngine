#pragma once
#include "SkrRT/config.h"
#include "SkrRT/resource/resource_handle.h"
#include "SkrRT/rttr/guid.hpp"
#include "SkrRT/rttr/type_desc.hpp"
#include "SkrRT/rttr/type_registry.hpp"
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/containers/sptr.hpp"
#include "SkrRT/containers/string.hpp"
#include "SkrRT/containers/variant.hpp"
#include "SkrRT/rttr/strongly_enum.hpp"

// RTTR traits
// 提供部分静态类型功能，从动态角度来说，实际上只是一层皮
namespace skr::rttr
{
struct Type;

template <typename T>
struct RTTRTraits {
    inline static constexpr size_t type_desc_size = 1;
    static void                    write_type_desc(TypeDesc* desc)
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "RTTRTraits<T>::write_type_desc() is not implemented");
#endif
    }

    static skr::StringView get_name()
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "RTTRTraits<T>::write_type_desc() is not implemented");
#else
        return {};
#endif
    }
    static GUID get_guid()
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "RTTRTraits<T>::write_type_desc() is not implemented");
#else
        return {};
#endif
    }
    static Type* get_type()
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "RTTRTraits<T>::write_type_desc() is not implemented");
#else
        return nullptr;
#endif
    }
};

template <typename T>
SKR_INLINE GUID type_id() SKR_NOEXCEPT
{
    return RTTRTraits<T>::get_guid();
}
template <typename T>
SKR_INLINE span<TypeDesc> type_desc() SKR_NOEXCEPT
{
    static TypeDesc desc[RTTRTraits<T>::type_desc_size];
    if (desc[0].type == ETypeDescType::SKR_TYPE_DESC_TYPE_VOID)
    {
        RTTRTraits<T>::write_type_desc(desc);
    }

    return { desc, RTTRTraits<T>::type_desc_size };
}
template <typename T>
SKR_INLINE skr::StringView type_name() SKR_NOEXCEPT
{
    return RTTRTraits<T>::get_name();
}
template <typename T>
SKR_INLINE Type* type_of() SKR_NOEXCEPT
{
    return RTTRTraits<T>::get_type();
}
} // namespace skr::rttr

// remove cv
namespace skr::rttr
{
template <typename T>
struct RTTRTraits<const T> : RTTRTraits<T> {
};
template <typename T>
struct RTTRTraits<volatile T> : RTTRTraits<T> {
};
} // namespace skr::rttr

// help marcos
#define SKR_RTTR_MAKE_U8(__VALUE) u8##__VALUE
#define SKR_RTTR_TYPE(__TYPE, __GUID)                                              \
    namespace skr::rttr                                                            \
    {                                                                              \
    template <>                                                                    \
    struct RTTRTraits<__TYPE> {                                                    \
        inline static constexpr size_t type_desc_size = 1;                         \
        inline static void             write_type_desc(TypeDesc* desc)             \
        {                                                                          \
            new (desc) TypeDesc{ get_guid() };                                     \
        }                                                                          \
                                                                                   \
        inline static skr::StringView get_name() { return SKR_RTTR_MAKE_U8(#__TYPE); } \
        inline static GUID        get_guid()                                       \
        {                                                                          \
            using namespace skr::guid::literals;                                   \
            return u8##__GUID##_guid;                                              \
        }                                                                          \
        inline static Type* get_type()                                             \
        {                                                                          \
            static Type* type = nullptr;                                           \
            if (!type)                                                             \
            {                                                                      \
                type = get_type_from_guid(get_guid());                             \
            }                                                                      \
            return type;                                                           \
        }                                                                          \
    };                                                                             \
    }

// primitive types
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

// pointer type
namespace skr::rttr
{
static constexpr inline GUID kPointerGenericGUID = SKR_CONSTEXPR_GUID("d2b6757d-3e32-4073-b483-62f41bc4bd8a");
template <typename T>
struct RTTRTraits<T*> {
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        new (desc) TypeDesc{ kPointerGenericGUID };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 1);
    }

    inline static skr::StringView get_name()
    {
        return u8"Pointer";
    }
    inline static GUID get_guid()
    {
        return get_type()->type_id();
    }
    inline static Type* get_type()
    {
        static Type* type = nullptr;
        if (!type)
        {
            TypeDesc desc[type_desc_size];
            write_type_desc(desc);

            type = get_type_from_type_desc({ desc, type_desc_size });
        }
        return type;
    }
};
} // namespace skr::rttr

// reference type
namespace skr::rttr
{
static constexpr inline GUID kReferenceGenericGUID = SKR_CONSTEXPR_GUID("277d6af2-91e9-4ee3-a56f-b6e3becf88df");
template <typename T>
struct RTTRTraits<T&> {
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        new (desc) TypeDesc{ kReferenceGenericGUID };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 1);
    }

    inline static skr::StringView get_name()
    {
        return u8"Reference";
    }
    inline static GUID get_guid()
    {
        return get_type()->type_id();
    }
    inline static Type* get_type()
    {
        static Type* type = nullptr;
        if (!type)
        {
            TypeDesc desc[type_desc_size];
            write_type_desc(desc);

            type = get_type_from_type_desc({ desc, type_desc_size });
        }
        return type;
    }
};
} // namespace skr::rttr

// array type
namespace skr::rttr
{
static constexpr inline GUID kArrayGenericGUID = SKR_CONSTEXPR_GUID("b382ee62-3a82-4cf0-9b29-263f2cc848d9");
template <typename T, size_t N>
struct RTTRTraits<T[N]> {
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 3;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        new (desc + 0) TypeDesc{ kArrayGenericGUID };
        new (desc + 1) TypeDesc{ uint64_t(1) };
        new (desc + 2) TypeDesc{ uint64_t(N) };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 3);
    }

    inline static skr::StringView get_name()
    {
        return u8"Array";
    }
    inline static GUID get_guid()
    {
        return get_type()->type_id();
    }
    inline static Type* get_type()
    {
        static Type* type = nullptr;
        if (!type)
        {
            TypeDesc desc[type_desc_size];
            write_type_desc(desc);

            type = get_type_from_type_desc({ desc, type_desc_size });
        }
        return type;
    }
};

template <typename T, size_t N1, size_t N2>
struct RTTRTraits<T[N1][N2]> {
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 4;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        new (desc + 0) TypeDesc{ kArrayGenericGUID };
        new (desc + 1) TypeDesc{ uint64_t(2) };
        new (desc + 2) TypeDesc{ uint64_t(N1) };
        new (desc + 3) TypeDesc{ uint64_t(N2) };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 4);
    }

    inline static skr::StringView get_name()
    {
        return u8"Array";
    }
    inline static GUID get_guid()
    {
        return get_type()->type_id();
    }
    inline static Type* get_type()
    {
        static Type* type = nullptr;
        if (!type)
        {
            TypeDesc desc[type_desc_size];
            write_type_desc(desc);

            type = get_type_from_type_desc({ desc, type_desc_size });
        }
        return type;
    }
};

template <typename T, size_t N1, size_t N2, size_t N3>
struct RTTRTraits<T[N1][N2][N3]> {
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 5;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        new (desc + 0) TypeDesc{ kArrayGenericGUID };
        new (desc + 1) TypeDesc{ uint64_t(3) };
        new (desc + 2) TypeDesc{ uint64_t(N1) };
        new (desc + 3) TypeDesc{ uint64_t(N2) };
        new (desc + 4) TypeDesc{ uint64_t(N3) };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 5);
    }

    inline static skr::StringView get_name()
    {
        return u8"Array";
    }
    inline static GUID get_guid()
    {
        return get_type()->type_id();
    }
    inline static Type* get_type()
    {
        static Type* type = nullptr;
        if (!type)
        {
            TypeDesc desc[type_desc_size];
            write_type_desc(desc);

            type = get_type_from_type_desc({ desc, type_desc_size });
        }
        return type;
    }
};

} // namespace skr::rttr

// skr types
// TODO. remove it
SKR_RTTR_TYPE(skr_guid_t, "80EE37B7-E9C0-40E6-BF2F-51E12053A7A9");
SKR_RTTR_TYPE(skr_md5_t, "F8ABEC14-9436-43B5-A93A-460E7D3CBEC2");
SKR_RTTR_TYPE(skr_resource_handle_t, "A9E0CE3D-5E9B-45F1-AC28-B882885C63AB");
SKR_RTTR_TYPE(skr_rotator_t, "236DFBE6-1554-4BCF-A021-1DAD18BF19B8");
SKR_RTTR_TYPE(skr_float2_t, "CD0FEAC8-C536-4DE7-854A-EC711E59F17D");
SKR_RTTR_TYPE(skr_float3_t, "9E6AB3E9-325F-4224-935C-233CC8DE47B6");
SKR_RTTR_TYPE(skr_float4_t, "38BFB8AD-2287-40FC-8AFE-185CABF84C4A");
SKR_RTTR_TYPE(skr_float4x4_t, "E49D2F13-9DFF-4A7B-8D43-27F68BA932E0");
SKR_RTTR_TYPE(skr_quaternion_t, "51977A88-7095-4FA7-8467-541698803936");
SKR_RTTR_TYPE(::skr::String, "214ED643-54BD-4213-BE37-E336A77FDE84");
SKR_RTTR_TYPE(::skr::StringView, "B799BA81-6009-405D-9131-E4B6101660DC");
SKR_RTTR_TYPE(::skr::SInterface, "244617fe-5274-47bc-aa3d-acd76dbbeddd");

// template types
// TODO. 仅仅为了过编译

namespace skr::rttr
{
template <typename T>
struct RTTRTraits<skr::Vector<T>> {
    inline static constexpr size_t type_desc_size = 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
    }

    inline static skr::StringView get_name()
    {
        return u8"Vector";
    }
    inline static GUID get_guid()
    {
        return {};
    }
    inline static Type* get_type()
    {
        return nullptr;
    }
};
template <typename T>
struct RTTRTraits<skr::span<T>> {
    inline static constexpr size_t type_desc_size = 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
    }

    inline static skr::StringView get_name()
    {
        return u8"Span";
    }
    inline static GUID get_guid()
    {
        return {};
    }
    inline static Type* get_type()
    {
        return nullptr;
    }
};
template <typename T>
struct RTTRTraits<skr::resource::TResourceHandle<T>> {
    inline static constexpr size_t type_desc_size = 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
    }

    inline static skr::StringView get_name()
    {
        return u8"ResourceHandle";
    }
    inline static GUID get_guid()
    {
        return {};
    }
    inline static Type* get_type()
    {
        return nullptr;
    }
};
template <typename T>
struct RTTRTraits<skr::SPtrHelper<T>> {
    inline static constexpr size_t type_desc_size = 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
    }

    inline static skr::StringView get_name()
    {
        return u8"SPtr";
    }
    inline static GUID get_guid()
    {
        return {};
    }
    inline static Type* get_type()
    {
        return nullptr;
    }
};
template <typename T>
struct RTTRTraits<skr::StronglyEnum<T>> {
    inline static constexpr size_t type_desc_size = 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
    }

    inline static skr::StringView get_name()
    {
        return u8"StronglyEnum";
    }
    inline static GUID get_guid()
    {
        return {};
    }
    inline static Type* get_type()
    {
        return nullptr;
    }
};
template <typename... TS>
struct RTTRTraits<skr::variant<TS...>> {
    inline static constexpr size_t type_desc_size = 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
    }

    inline static skr::StringView get_name()
    {
        return u8"Variant";
    }
    inline static GUID get_guid()
    {
        return {};
    }
    inline static Type* get_type()
    {
        return nullptr;
    }
};
}; // namespace skr::rttr