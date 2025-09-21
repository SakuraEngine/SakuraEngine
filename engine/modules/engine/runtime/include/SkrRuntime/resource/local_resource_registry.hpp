#pragma once
#include "resource_system.h"

struct skr_vfs_t;
namespace skr
{
struct SKR_RUNTIME_API LocalResourceRegistry : ResourceRegistry
{
    LocalResourceRegistry(skr_vfs_t* vfs);
    virtual ~LocalResourceRegistry() = default;
    bool RequestResourceFile(ResourceRequest* request) override;
    void CancelRequestFile(ResourceRequest* requst) override;
    skr_vfs_t* vfs;
};
} // namespace skr