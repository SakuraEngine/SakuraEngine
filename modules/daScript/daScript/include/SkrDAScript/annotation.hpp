#pragma once
#include "SkrDAScript/type.hpp"

namespace skr {
namespace das {

struct SKR_DASCRIPT_API Annotation
{
    virtual ~Annotation() SKR_NOEXCEPT;
    virtual void* get_ptrptr() SKR_NOEXCEPT = 0;
};

struct SKR_DASCRIPT_API TypeAnnotation : public Annotation
{
    virtual ~TypeAnnotation() SKR_NOEXCEPT;

};

enum class EBuiltinType : uint32_t
{
    BITFIELD,
    UINT8,
    INT8,
    UINT16,
    INT16,
    UINT32,
    INT32,
    UINT64,
    INT64,
    FLOAT,
    DOUBLE,
    VOID,
    PTR,
    ENUMERATION,
    ENUMERATION8,
    ENUMERATION16,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    UINT2,
    INT2,
    UINT3,
    INT3,
    UINT4,
    INT4,

    URANGE,
    RANGE,
    URANGE64,
    RANGE64,

    ARRAY,
    TABLE,
    BLOCK,
    FUNCTION,
    LAMBDA,
    TUPLE,
    VARIANT
};

struct EStructureAnnotationFlag
{
    enum {
        None = 0,
        HasNonTrivalCtor = 1 << 0,
        HasNonTrivalDtor = 1 << 1,
        HasNonTrivalCopy = 1 << 2,
        IsPOD = 1 << 3,
        CanClone = 1 << 4,
        CanNew = 1 << 5,
        CanDeletePtr = 1 << 6
    };
};
using StructureAnnotationFlags = uint32_t;

template <typename T>
struct is_cloneable  
{
    template<typename U>
    static decltype(declval<U&>() = declval<const U&>(), U (declval<const U&>()), std::true_type{}) func (std::remove_reference_t<U>*);
    template<typename U>
    static std::false_type func (...);
    using  type = decltype(func<T>(nullptr));
    static constexpr bool value { type::value };
};

struct StructureAnnotationDescriptor
{
    const char8_t* name = nullptr;
    const char8_t* cppname = nullptr;
    uint64_t size = 0;
    uint64_t alignment = 0;
    void (*initializer)(void* ptr) = nullptr;
    void (*finalizer)(void* ptr) = nullptr;
    StructureAnnotationFlags flags = EStructureAnnotationFlag::None;
};

struct SKR_DASCRIPT_API StructureAnnotation : public TypeAnnotation
{
    static StructureAnnotation* Create(Library* library, const StructureAnnotationDescriptor& desc) SKR_NOEXCEPT;
    
    template<typename T>
    inline static StructureAnnotation* Create(Library* library, const char8_t* name, const char8_t* cppname = nullptr)
    {
        skr::das::StructureAnnotationDescriptor AnnotationDesc = {};
        AnnotationDesc.name = name;
        AnnotationDesc.cppname = cppname ? cppname : name;
        AnnotationDesc.size = sizeof(T);
        AnnotationDesc.alignment = alignof(T);
        AnnotationDesc.initializer = +[](void* ptr) { new (ptr) T; };
        AnnotationDesc.finalizer = +[](void* ptr) { delete (T*)ptr; };
        if constexpr (!std::is_trivially_constructible<T>::value) 
        {
            AnnotationDesc.flags |= skr::das::EStructureAnnotationFlag::HasNonTrivalCtor;
        }
        if constexpr (!std::is_trivially_destructible<T>::value) 
        {
            AnnotationDesc.flags |= skr::das::EStructureAnnotationFlag::HasNonTrivalDtor;
        }
        if constexpr (!std::is_trivially_copyable_v<T> || !std::is_trivially_copy_constructible_v<T>) 
        {
            AnnotationDesc.flags |= skr::das::EStructureAnnotationFlag::HasNonTrivalCopy;
        }
        if constexpr (std::is_pod<T>::value) 
        {
            AnnotationDesc.flags |= skr::das::EStructureAnnotationFlag::IsPOD;
        }
        if constexpr (skr::das::is_cloneable<T>::value) 
        {
            AnnotationDesc.flags |= skr::das::EStructureAnnotationFlag::CanClone;
        }
        AnnotationDesc.flags |= skr::das::EStructureAnnotationFlag::CanNew;
        AnnotationDesc.flags |= skr::das::EStructureAnnotationFlag::CanDeletePtr;

        return Create(library, AnnotationDesc);
    }
    
    static void Free(StructureAnnotation* annotation) SKR_NOEXCEPT;

    virtual ~StructureAnnotation() SKR_NOEXCEPT;
    virtual void add_field(uint32_t offset, EBuiltinType type, const char8_t* na, const char8_t* cppna = nullptr) SKR_NOEXCEPT = 0;
    virtual void add_field(uint32_t offset, TypeDecl* typedecl, const char8_t* na, const char8_t* cppna = nullptr) SKR_NOEXCEPT = 0;
};

} // namespace das
} // namespace skr