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
    UINT8,
    UINT16,
    UINT32,
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