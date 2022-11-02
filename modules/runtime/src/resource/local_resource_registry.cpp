#include <platform/filesystem.hpp>
#include "utils/defer.hpp"
#include "utils/format.hpp"
#include "platform/vfs.h"
#include "resource/local_resource_registry.h"
#include "resource/resource_header.h"

namespace skr::resource
{
SLocalResourceRegistry::SLocalResourceRegistry(skr_vfs_t* vfs)
    : vfs(vfs)
{
}
bool SLocalResourceRegistry::RequestResourceFile(SResourceRequest* request)
{
    //简单实现，直接在 resource 路径下按 guid 找到文件读信息，没有单独的数据库
    auto guid = request->resourceRecord->header.guid;
    skr::filesystem::path path = fmt::format("{}.bin", guid);
    // TODO: 检查文件存在？
    auto file = skr_vfs_fopen(vfs, path.u8string().c_str(), SKR_FM_READ, SKR_FILE_CREATION_OPEN_EXISTING);
    if (!file) return false;
    char buffer[sizeof(skr_resource_header_t)];
    skr_vfs_fread(file, buffer, 0, sizeof(skr_resource_header_t));
    SKR_DEFER({ skr_vfs_fclose(file); });
    // TODO: 把这些封装起来传过去少暴露一些细节？
    request->resourceRecord->header.type = ((skr_resource_header_t*)buffer)->type;
    request->resourceRecord->header.version = ((skr_resource_header_t*)buffer)->version;
    request->path = path;
    request->vfs = vfs;
    request->OnRequestFileFinished();
    return true;
}

void SLocalResourceRegistry::CancelRequestFile(SResourceRequest* requst)
{

}
} // namespace skr::resource