#include "resource/local_resource_registry.h"
#include "ghc/filesystem.hpp"
#include "platform/guid.h"
#include "platform/vfs.h"
#include "utils/defer.hpp"
#include "utils/format.hpp"
#include "resource/resource_header.h"

namespace skr::resource
{
SLocalResourceRegistry::SLocalResourceRegistry(skr_vfs_t* vfs)
    : vfs(vfs)
{
}
void SLocalResourceRegistry::RequestResourceFile(SResourceRequest* request)
{
    auto guid = request->resourceRecord->header.guid;
    ghc::filesystem::path path = fmt::format("{}.bin", guid);
    // TODO: 检查文件存在？
    auto file = skr_vfs_fopen(vfs, path.u8string().c_str(), SKR_FM_READ, SKR_FILE_CREATION_OPEN_EXISTING);
    char buffer[sizeof(skr_resource_header_t)];
    skr_vfs_fread(file, buffer, 0, sizeof(skr_resource_header_t));
    SKR_DEFER({ skr_vfs_fclose(file); });
    request->resourceRecord->header.type = ((skr_resource_header_t*)buffer)->type;
    request->resourceRecord->header.version = ((skr_resource_header_t*)buffer)->version;
    request->path = path;
    request->OnRequestFileFinished();
}
void SLocalResourceRegistry::CancelRequestFile(SResourceRequest* requst)
{
}
} // namespace skr::resource