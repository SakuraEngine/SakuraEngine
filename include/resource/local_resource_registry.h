#include "ghc/filesystem.hpp"
#include "platform/guid.h"
#include "resource_system.h"
#include "utils/hash.h"
#include "utils/hashmap.hpp"
struct skr_vfs_t;
namespace skr::resource
{
struct RUNTIME_API SLocalResourceRegistry : SResourceRegistry {
    SLocalResourceRegistry(skr_vfs_t* vfs);
    void RequestResourceFile(SResourceRequest* request) override;
    void CancelRequestFile(SResourceRequest* requst) override;
    skr_vfs_t* vfs;
};
} // namespace skr::resource