#include <platform/filesystem.hpp>
#include "utils/defer.hpp"
#include "platform/vfs.h"
#include "resource/local_resource_registry.hpp"
#include "resource/resource_header.hpp"
#include "utils/log.hpp"
#include "binary/reader.h"
#include "platform/guid.hpp"

namespace skr::resource
{
SLocalResourceRegistry::SLocalResourceRegistry(skr_vfs_t* vfs)
    : vfs(vfs)
{

}

bool SLocalResourceRegistry::RequestResourceFile(SResourceRequest* request)
{
    //简单实现，直接在 resource 路径下按 guid 找到文件读信息，没有单独的数据库
    auto guid = request->GetGuid();
    skr::filesystem::path headerPath = fmt::format("game/{}.rh", guid);
    auto headerUri = headerPath.u8string();
    // TODO: 检查文件存在？
    auto file = skr_vfs_fopen(vfs, headerUri.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    if (!file) return false;
    SKR_DEFER({ skr_vfs_fclose(file); });
    uint8_t buffer[sizeof(skr_resource_header_t)];
    uint32_t _fs_length = (uint32_t)skr_vfs_fsize(file);
    if(skr_vfs_fread(file, buffer, 0, _fs_length) != _fs_length)
    {
        SKR_LOG_FMT_ERROR("[SLocalResourceRegistry::RequestResourceFile] failed to read resource header! guid: {}", guid);
        return false;
    }
    skr::binary::SpanReader reader = {buffer};
    skr_binary_reader_t archive{reader};
    skr_resource_header_t header;
    if(skr::binary::Read(&archive, header) != 0)
        return false;
    SKR_ASSERT(header.guid == guid);
    auto resourcePath = headerPath;
    resourcePath.replace_extension(".bin");
    auto resourceUri = resourcePath.u8string();
    FillRequest(request, header, vfs, resourceUri.c_str());
    request->OnRequestFileFinished();
    return true;
}

void SLocalResourceRegistry::CancelRequestFile(SResourceRequest* requst)
{

}
} // namespace skr::resource