#include "types.hpp"

namespace skr {
namespace das {

TypeDecl* TypeDecl::CreateHandle(Library* lib, const char8_t* name) SKR_NOEXCEPT
{
    return SkrNew<TypeDeclImpl>(lib, name);
}

void TypeDecl::Free(TypeDecl* decl) SKR_NOEXCEPT
{
    SkrDelete(decl);
}

TypeDecl::~TypeDecl() SKR_NOEXCEPT
{

}

TypeDeclImpl::TypeDeclImpl(Library* lib, const char8_t* name) SKR_NOEXCEPT
    : decl(
        ::das::makeHandleType(static_cast<LibraryImpl*>(lib)->libGroup, (const char*)name)
    )
{

}

} // namespace das
} // namespace skr