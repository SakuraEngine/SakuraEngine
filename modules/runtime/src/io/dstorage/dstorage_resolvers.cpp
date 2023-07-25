#include "../../pch.hpp"
#include "SkrRT/platform/vfs.h"
#include <SkrRT/platform/filesystem.hpp>
#include "../common/io_request.hpp"
#include "dstorage_resolvers.hpp"

namespace skr {
namespace io {

void DStorageFileResolver::resolve(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT
{
    if (auto pFile = io_component<IOFileComponent>(request.get()))
    {
        if (!pFile->dfile)
        {
            ZoneScopedN("DStorage::OpenFile");

            SKR_ASSERT(pFile->vfs);
            skr::filesystem::path p = pFile->vfs->mount_dir;
            p /= pFile->path.c_str();

            auto instance = skr_get_dstorage_instnace();
            pFile->dfile = skr_dstorage_open_file(instance, p.string().c_str());
            SKR_ASSERT(pFile->dfile);
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }
}

} // namespace io
} // namespace skr