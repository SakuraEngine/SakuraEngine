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

struct SKR_DASCRIPT_API StructureAnnotation : public TypeAnnotation
{
    static StructureAnnotation* Create(const char8_t* name, const char8_t* cppname, Library* library) SKR_NOEXCEPT;
    static void Free(StructureAnnotation* annotation) SKR_NOEXCEPT;

    virtual ~StructureAnnotation() SKR_NOEXCEPT;
    virtual void add_field(const char8_t* na, const char8_t* cppna, uint32_t offset, EBuiltinType type) SKR_NOEXCEPT = 0;
    virtual void add_field(const char8_t* na, const char8_t* cppna, uint32_t offset, TypeDecl* typedecl) SKR_NOEXCEPT = 0;
};

} // namespace das
} // namespace skr