#include "gamert.h"
#include "ghc/filesystem.hpp"

skr_vfs_t* skg::Core::resource_vfs = nullptr;

void skg::Core::initialize()
{
    auto resourceRoot = (ghc::filesystem::current_path() / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    skg::Core::resource_vfs = skr_create_vfs(&vfs_desc);
}
