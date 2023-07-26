#include "../../pch.hpp"
#include "SkrRT/platform/vfs.h"
#include <SkrRT/platform/filesystem.hpp>
#include "../common/io_request.hpp"
#include "../common/io_batch.hpp"
#include "dstorage_resolvers.hpp"

namespace skr {
namespace io {

void DStorageFileResolver::resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT
{
    auto B = static_cast<IOBatchBase*>(batch.get());
    auto pFile = io_component<FileSrcComponent>(request.get());
    if (!B->can_use_dstorage) 
        return;

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

} // namespace io
} // namespace skr