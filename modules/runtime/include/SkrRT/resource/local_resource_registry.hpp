#pragma once
#include "SkrRT/misc/types.h"
#include "resource_system.h"

struct skr_vfs_t;
namespace skr::resource
{
struct SKR_RUNTIME_API SLocalResourceRegistry : SResourceRegistry {
    SLocalResourceRegistry(skr_vfs_t* vfs);
    virtual ~SLocalResourceRegistry() = default;
    bool RequestResourceFile(SResourceRequest* request) override;
    void CancelRequestFile(SResourceRequest* requst) override;
    skr_vfs_t* vfs;
};
} // namespace skr::resource