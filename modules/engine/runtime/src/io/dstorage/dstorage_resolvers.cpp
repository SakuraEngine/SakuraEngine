#include "SkrCore/platform/vfs.h"
#include <SkrOS/filesystem.hpp>
#include "../common/io_request.hpp"
#include "../common/io_batch.hpp"
#include "dstorage_resolvers.hpp"

namespace skr {
namespace io {

void DStorageFileResolver::resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT
{
    auto B = static_cast<IOBatchBase*>(batch.get());
    auto pPath = io_component<PathSrcComponent>(request.get());
    auto pFile = io_component<FileComponent>(request.get());
    if (!B->can_use_dstorage) 
        return;

    if (pPath && !pFile->dfile)
    {
        SkrZoneScopedN("DStorage::OpenFile");

        SKR_ASSERT(pPath->get_vfs());
        skr::filesystem::path p = pPath->get_vfs()->mount_dir;
        p /= pPath->get_path();

        auto instance = skr_get_dstorage_instnace();
        pFile->dfile = skr_dstorage_open_file(instance, p.u8string().c_str());
        SKR_ASSERT(pFile->dfile);
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
}

} // namespace io
} // namespace skr