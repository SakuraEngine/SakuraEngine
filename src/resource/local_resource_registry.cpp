#include "resource/local_resource_registry.h"
#include "ghc/filesystem.hpp"
#include "platform/guid.h"
#include "platform/vfs.h"
#include "utils/defer.hpp"
#include "resource/resource_header.h"

namespace skr::resource
{
SLocalResourceRegistry::SLocalResourceRegistry(ghc::filesystem::path root)
    : rootDirectory(root)
{
    skr_vfs_desc_t abs_fs_desc = {};
    abs_fs_desc.app_name = "fs-test";
    abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    abs_fs = skr_create_vfs(&abs_fs_desc);
    ghc::filesystem::recursive_directory_iterator iter(root);
    for (auto& entry : iter)
    {
        if (entry.is_regular_file())
        {
            auto filename = entry.path().filename().string();
            if (filename.size() == 34 && filename[8] == '-' && filename[13] == '-' && filename[18] == '-')
            {
                auto guid = skr::guid::make_guid(filename);
                resources.insert(std::make_pair(guid, entry.path()));
            }
        }
    }
}
void SLocalResourceRegistry::RequestResourceFile(SResourceRequest* request)
{
    auto guid = request->resourceRecord->header.guid;
    auto iter = resources.find(guid);
    if (iter != resources.end())
    {
        auto file = skr_vfs_fopen(abs_fs, iter->second.u8string().c_str(), SKR_FM_READ, SKR_FILE_CREATION_OPEN_EXISTING);
        char buffer[sizeof(skr_resource_header_t)];
        skr_vfs_fread(file, buffer, 0, sizeof(skr_resource_header_t));
        SKR_DEFER({ skr_vfs_fclose(file); });
        request->resourceRecord->header.type = ((skr_resource_header_t*)buffer)->type;
        request->resourceRecord->header.version = ((skr_resource_header_t*)buffer)->version;
        request->path = iter->second;
    }
    request->OnRequestFileFinished();
}
void SLocalResourceRegistry::CancelRequestFile(SResourceRequest* requst)
{
}
} // namespace skr::resource