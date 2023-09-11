#pragma once
#include "SkrRT/platform/configure.h"
#include "SkrRT/rttr/guid.hpp"
#include "SkrRT/rttr/type_desc.hpp"
#include "SkrRT/rttr/type_registry.hpp"
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/containers/string.hpp"

// RTTR traits
// 提供部分静态类型功能，从动态角度来说，实际上只是一层皮
namespace skr::rttr
{
struct Type;

template <typename T>
struct RTTRTraits {
    inline static constexpr size_t type_desc_size = 1;
    static void                    write_type_desc(TypeDesc* desc);

    static string_view get_name();
    static GUID        get_guid();
    static Type*       get_type();
};

template <typename T>
SKR_INLINE GUID type_id() SKR_NOEXCEPT
{
    return RTTRTraits<T>::get_guid();
}
template <typename T>
SKR_INLINE Span<TypeDesc> type_desc() SKR_NOEXCEPT
{
    static TypeDesc desc[RTTRTraits<T>::type_desc_size];
    if (desc[0].type == ETypeDescType::SKR_TYPE_DESC_TYPE_VOID)
    {
        RTTRTraits<T>::write_type_desc(desc);
    }

    return { desc, RTTRTraits<T>::type_desc_size };
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

// primitive types
namespace skr::rttr
{
#define SKR_RTTR_PRIMITIVE_MAKE_U8(__VALUE) u8##__VALUE
#define SKR_RTTR_PRIMITIVE_TYPE_TRAITS(__TYPE, __GUID)                                       \
    template <>                                                                              \
    struct RTTRTraits<__TYPE> {                                                              \
        inline static constexpr size_t type_desc_type = 1;                                   \
        inline static void             write_type_desc(TypeDesc* desc)                       \
        {                                                                                    \
            new (desc) TypeDesc{ get_guid() };                                               \
        }                                                                                    \
                                                                                             \
        inline static string_view get_name() { return SKR_RTTR_PRIMITIVE_MAKE_U8(#__TYPE); } \
        inline static GUID        get_guid() { return __GUID##_guid; }                       \
        inline static Type*       get_type()                                                 \
        {                                                                                    \
            static Type* type = nullptr;                                                     \
            if (!type)                                                                       \
            {                                                                                \
                type = get_type_from_guid(get_guid());                                       \
            }                                                                                \
            return type;                                                                     \
        }                                                                                    \
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
#undef SKR_RTTR_PRIMITIVE_MAKE_U8
} // namespace skr::rttr

// pointer type
namespace skr::rttr
{
static constexpr inline GUID kPointerGenericGUID = "d2b6757d-3e32-4073-b483-62f41bc4bd8a"_guid;
template <typename T>
struct RTTRTraits<T*> {
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        new (desc) TypeDesc{ kPointerGenericGUID };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 1);
    }

    inline static string_view get_name()
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
static constexpr inline GUID kReferenceGenericGUID = "277d6af2-91e9-4ee3-a56f-b6e3becf88df"_guid;
template <typename T>
struct RTTRTraits<T&> {
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        new (desc) TypeDesc{ kReferenceGenericGUID };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 1);
    }

    inline static string_view get_name()
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
static constexpr inline GUID kArrayGenericGUID = "b382ee62-3a82-4cf0-9b29-263f2cc848d9"_guid;
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

    inline static string_view get_name()
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
struct RTTRTraits<T[N1][N2]>
{
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 4;
    inline static void             write_type_desc(TypeDesc * desc)
    {
        new (desc + 0) TypeDesc{ kArrayGenericGUID };
        new (desc + 1) TypeDesc{ uint64_t(2) };
        new (desc + 2) TypeDesc{ uint64_t(N1) };
        new (desc + 3) TypeDesc{ uint64_t(N2) };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 4);
    }

    inline static string_view get_name()
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
struct RTTRTraits<T[N1][N2][N3]>
{
    inline static constexpr size_t type_desc_size = RTTRTraits<std::remove_cv_t<T>>::type_desc_size + 5;
    inline static void             write_type_desc(TypeDesc * desc)
    {
        new (desc + 0) TypeDesc{ kArrayGenericGUID };
        new (desc + 1) TypeDesc{ uint64_t(3) };
        new (desc + 2) TypeDesc{ uint64_t(N1) };
        new (desc + 3) TypeDesc{ uint64_t(N2) };
        new (desc + 4) TypeDesc{ uint64_t(N3) };
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 5);
    }

    inline static string_view get_name()
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