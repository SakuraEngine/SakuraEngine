#pragma once
#include "SkrGuid/guid.hpp"
#include "SkrBase/config.h"
#include "SkrRTTR/type_desc.hpp"
#include "SkrRTTR/type_registry.hpp"
#include "SkrRTTR/type/type.hpp"
#include "SkrContainers/sptr.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/variant.hpp"
#include "SkrRTTR/strongly_enum.hpp"
#include "SkrBase/meta.h"

// RTTR traits
// 提供部分静态类型功能，从动态角度来说，实际上只是一层皮
namespace skr::rttr
{
struct Type;

template <typename T>
struct RTTRTraits {
    // TODO. type_desc_size 和 write_type_desc 是常驻选项
    // TODO. get_guid 和 get_name 仅在非 GenericType 时有意义
    // TODO. 不提供 get_type，对于具体行为应该由获取 type_desc 的一方自行实现

    inline static constexpr size_t type_desc_size = 1;
    static void                    write_type_desc(TypeDesc* desc)
    {
        unimplemented_no_meta(T, "RTTRTraits<T>::write_type_desc() is not implemented");
    }

    static skr::StringView get_name()
    {
        unimplemented_no_meta(T, "RTTRTraits<T>::get_name() is not implemented");
        return {};
    }
    static GUID get_guid()
    {
        unimplemented_no_meta(T, "RTTRTraits<T>::get_guid() is not implemented");
        return {};
    }
    static Type* get_type()
    {
        unimplemented_no_meta(T, "RTTRTraits<T>::get_type() is not implemented");
        return {};
    }
};

template <typename T>
SKR_INLINE GUID type_id() SKR_NOEXCEPT
{
    return RTTRTraits<T>::get_guid();
}
// TODO. 删掉这玩意，直接返回 Vector 堆内存对象
template <typename T>
SKR_INLINE span<TypeDesc> type_desc() SKR_NOEXCEPT
{
    static TypeDesc desc[RTTRTraits<T>::type_desc_size];
    if (desc[0].type() == ETypeDescType::SKR_TYPE_DESC_TYPE_VOID)
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

//======================================== modifier
namespace skr::rttr
{
// ignore volatile
template <typename T>
struct RTTRTraits<volatile T> : RTTRTraits<T> {
};

template <typename T>
struct RTTRTraits<const T> {
    inline static constexpr size_t type_desc_size = RTTRTraits<T>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        desc[0].set_const();
        RTTRTraits<T>::write_type_desc(desc + 1);
    }

    inline static skr::StringView get_name()
    {
        return u8"Const";
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

template <typename T>
struct RTTRTraits<T*> {
    inline static constexpr size_t type_desc_size = RTTRTraits<T>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        desc[0].set_pointer();
        RTTRTraits<T>::write_type_desc(desc + 1);
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

template <typename T>
struct RTTRTraits<T&> {
    inline static constexpr size_t type_desc_size = RTTRTraits<T>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        desc[0].set_ref();
        RTTRTraits<T>::write_type_desc(desc + 1);
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

template <typename T>
struct RTTRTraits<T&&> {
    inline static constexpr size_t type_desc_size = RTTRTraits<T>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        desc[0].set_rvalue_ref();
        RTTRTraits<T>::write_type_desc(desc + 1);
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

template <typename T, size_t N>
struct RTTRTraits<T[N]> {
    inline static constexpr size_t type_desc_size = RTTRTraits<T>::type_desc_size + 1;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        desc[0].set_array_dim(N);
        RTTRTraits<std::remove_cv_t<T>>::write_type_desc(desc + 1);
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

template <typename T, size_t N>
struct RTTRTraits<const T[N]> {
    inline static constexpr size_t type_desc_size = RTTRTraits<T>::type_desc_size + 2;
    inline static void             write_type_desc(TypeDesc* desc)
    {
        desc[0].set_const();
        desc[0].set_array_dim(N);
        RTTRTraits<const T>::write_type_desc(desc + 1);
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

//======================================== register marco
// TODO. new register marcos
// SKR_RTTR_RECORD(XXX, "6b51fd29-ea47-4ec4-9003-7f45d8d2deed")
// {
//    using namespace skr::rttr;
//    Class_<XXX>()
//        .method<&XXX::xxx>()
//        .field<&XXX::xxx>();
// }
// SKR_RTTR_PRIMITIVE(int32_t, "3ca87da3-d240-4c45-8ce6-1dc0a6c443d6")
// SKR_RTTR_ENUM(ETest, "bc49b336-49fa-452c-84b4-143831cf1a8c")
// {
//     ... 如果是手动注册，可以在这里写注册代码，否则这里永远不会被触达
//     抛弃静态转换函数，在触及性能热点时再考虑
//     // code gen use skr::rttr::EnumTraits for fast enum reflection
//     SKR_UNREACHABLE_CODE();
// }
#define SKR_RTTR_PRIMITIVE(__TYPE, __GUID)
#define SKR_RTTR_RECORD(__TYPE, __GUID)
#define SKR_RTTR_ENUM(__TYPE, __GUID)

// help marcos
#define SKR_RTTR_MAKE_U8(__VALUE) u8##__VALUE
#define SKR_RTTR_TYPE(__TYPE, __GUID)                                                  \
    namespace skr::rttr                                                                \
    {                                                                                  \
    template <>                                                                        \
    struct RTTRTraits<__TYPE> {                                                        \
        inline static constexpr size_t type_desc_size = 1;                             \
        inline static void             write_type_desc(TypeDesc* desc)                 \
        {                                                                              \
            desc[0].set_type_id(get_guid());                                           \
        }                                                                              \
                                                                                       \
        inline static skr::StringView get_name() { return SKR_RTTR_MAKE_U8(#__TYPE); } \
        inline static GUID            get_guid()                                       \
        {                                                                              \
            using namespace skr::guid::literals;                                       \
            return u8##__GUID##_guid;                                                  \
        }                                                                              \
        inline static Type* get_type()                                                 \
        {                                                                              \
            static Type* type = nullptr;                                               \
            if (!type)                                                                 \
            {                                                                          \
                type = get_type_from_guid(get_guid());                                 \
            }                                                                          \
            return type;                                                               \
        }                                                                              \
    };                                                                                 \
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

//======================================== skr template types
// TODO. 仅仅为了过编译, 后续实现具体类型
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