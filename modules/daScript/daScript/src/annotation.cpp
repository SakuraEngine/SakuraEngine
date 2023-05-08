#include "SkrDAScript/annotation.hpp"
#include "types.hpp"

namespace skr {
namespace das {

Annotation::~Annotation() SKR_NOEXCEPT {}
TypeAnnotation::~TypeAnnotation() SKR_NOEXCEPT {}

StructureAnnotation* StructureAnnotation::Create(const char8_t* name, const char8_t* cppname, Library* library) SKR_NOEXCEPT
{
    return SkrNew<StructureAnnotationImpl>(name, cppname, library);
}

void StructureAnnotation::Free(StructureAnnotation* annotation) SKR_NOEXCEPT
{
    SkrDelete(annotation);
}

StructureAnnotation::~StructureAnnotation() SKR_NOEXCEPT { }


StructureAnnotationImpl::StructureAnnotationImpl(
    const char8_t* name, const char8_t* cppname, Library* library) SKR_NOEXCEPT
    : Lib(static_cast<LibraryImpl*>(library)), 
      annotation(::das::make_smart<Structure>(
        (const char*)name, (const char*)cppname, 
        &static_cast<LibraryImpl*>(library)->libGroup)
    )
{

}

void StructureAnnotationImpl::add_field(const char8_t* na, const char8_t* cppna, uint32_t offset, TypeDecl* typedecl) SKR_NOEXCEPT
{
    auto Decl = static_cast<TypeDeclImpl*>(typedecl);
    annotation->addFieldEx((const char*)na, (const char*)cppna, offset, Decl->decl);
}

void StructureAnnotationImpl::add_field(const char8_t* na, const char8_t* cppna, uint32_t offset, EBuiltinType type) SKR_NOEXCEPT
{
    annotation->addFieldEx((const char*)na, (const char*)cppna, offset, ::das::makeType<uint8_t>(Lib->libGroup));
}


} // namespace das
} // namespace skr