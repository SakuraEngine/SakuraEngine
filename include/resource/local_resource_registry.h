#include "ghc/filesystem.hpp"
#include "platform/guid.h"
#include "resource_system.h"
#include "utils/hash.h"
#include "utils/hashmap.hpp"
struct skr_vfs_t;
namespace skr::resource
{
struct RUNTIME_API SLocalResourceRegistry : SResourceRegistry {
    SLocalResourceRegistry(ghc::filesystem::path root);
    void RequestResourceFile(SResourceRequest* request) override;
    void CancelRequestFile(SResourceRequest* requst) override;
    ghc::filesystem::path rootDirectory;
    skr::flat_hash_map<skr_guid_t, ghc::filesystem::path, skr::guid::hash> resources;
    skr_vfs_t* abs_fs;
};
} // namespace skr::resource