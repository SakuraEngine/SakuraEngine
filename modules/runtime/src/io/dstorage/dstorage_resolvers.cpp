#include "../../pch.hpp"
#include "../common/io_resolver.hpp"
#include "../common/io_request.hpp"
#include "SkrRT/platform/vfs.h"

namespace skr {
namespace io {

struct DStorageFileResolver : public IORequestResolverBase
{
    virtual void resolve(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT
    {
        auto rq = skr::static_pointer_cast<IORequestBase>(request);
        if (!rq->dfile)
        {
            ZoneScopedN("DStorage::OpenFile");

            SKR_ASSERT(rq->vfs);
            skr::filesystem::path p = rq->vfs->mount_dir;
            p /= rq->path.c_str();

            auto instance = skr_get_dstorage_instnace();
            rq->dfile = skr_dstorage_open_file(instance, p.string().c_str());
            SKR_ASSERT(rq->dfile);
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }
};

IORequestResolverId create_dstorage_file_resolver() SKR_NOEXCEPT
{ 
    return SObjectPtr<DStorageFileResolver>::Create();
}

} // namespace io
} // namespace skr