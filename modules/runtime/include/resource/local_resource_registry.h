#pragma once
#include "ghc/filesystem.hpp"
#include "utils/types.h"
#include "resource_system.h"
#include "utils/hash.h"
#include "utils/hashmap.hpp"

struct skr_vfs_t;
namespace skr::resource
{
struct RUNTIME_API SLocalResourceRegistry : SResourceRegistry {
    SLocalResourceRegistry(skr_vfs_t* vfs);
    virtual ~SLocalResourceRegistry() = default;
    void RequestResourceFile(SResourceRequest* request) override;
    void CancelRequestFile(SResourceRequest* requst) override;
    skr_vfs_t* vfs;
};
} // namespace skr::resource