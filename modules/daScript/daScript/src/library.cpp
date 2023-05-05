#include "types.hpp"

namespace skr {
namespace das {

Library::~Library() SKR_NOEXCEPT
{

}

Library* Library::Create(const LibraryDescriptor& desc) SKR_NOEXCEPT
{
    return SkrNew<LibraryImpl>();
}

void Library::Free(Library* library) SKR_NOEXCEPT
{
    SkrDelete(library);
}

} // namespace das
} // namespace skr