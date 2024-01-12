#include "types.hpp"

namespace skr {
namespace das {

FileAccess::~FileAccess() SKR_NOEXCEPT
{

}

FileAccess* FileAccess::Create(const FileAccessDescriptor& desc) SKR_NOEXCEPT
{
    return SkrNew<FileAccessImpl>();
}

void FileAccess::Free(FileAccess* faccess) SKR_NOEXCEPT
{
    SkrDelete(faccess);
}

} // namespace das
} // namespace skr