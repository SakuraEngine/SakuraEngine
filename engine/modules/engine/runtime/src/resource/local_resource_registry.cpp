#include "SkrBase/misc/defer.hpp"
#include "SkrCore/log.hpp"
#include "SkrCore/platform/vfs.h"
#include "SkrRuntime/resource/local_resource_registry.hpp"
#include "SkrCore/serialize/binary_archive.hpp"
#include "SkrRuntime/resource/resource_header.hpp"
#include "SkrContainers/span.hpp"
#include "SkrContainersDef/path.hpp"

namespace skr
{
LocalResourceRegistry::LocalResourceRegistry(skr_vfs_t* vfs)
    : vfs(vfs)
{
}

bool LocalResourceRegistry::RequestResourceFile(ResourceRequest* request)
{
    // 简单实现，直接在 resource 路径下按 guid 找到文件读信息，没有单独的数据库
    auto guid = request->GetGuid();
    auto headerPath = skr::format(u8"{}.rh", guid);
    auto headerUri = headerPath;
    // TODO: 检查文件存在？
    SKR_LOG_BACKTRACE(u8"Failed to find resource file: %s!", headerUri.c_str());
    auto file = skr_vfs_fopen(vfs, headerUri.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    if (!file) return false;
    SKR_DEFER({ skr_vfs_fclose(file); });
    uint32_t _fs_length = (uint32_t)skr_vfs_fsize(file);
    SResourceHeader header;
    if (_fs_length <= sizeof(SResourceHeader))
    {
        uint8_t buffer[sizeof(SResourceHeader)];
        if (skr_vfs_fread(file, buffer, 0, _fs_length) != _fs_length)
        {
            SKR_LOG_FMT_ERROR(u8"[LocalResourceRegistry::RequestResourceFile] failed to read resource header! guid: {}", guid);
            return false;
        }

        skr::ArReadBin reader;
        reader.use_buffer(buffer);
        reader.value(header);
        SKR_FAST_CHECK(reader.is_succeeded(), false);
    }
    else
    {
        uint8_t* buffer = (uint8_t*)sakura_malloc(_fs_length);
        SKR_DEFER({ sakura_free(buffer); });
        if (skr_vfs_fread(file, buffer, 0, _fs_length) != _fs_length)
        {
            SKR_LOG_FMT_ERROR(u8"[LocalResourceRegistry::RequestResourceFile] failed to read resource header! guid: {}", guid);
            return false;
        }

        skr::ArReadBin reader;
        reader.use_buffer({ buffer, _fs_length });
        reader.value(header);
        SKR_FAST_CHECK(reader.is_succeeded(), false);
    }
    SKR_ASSERT(header.guid == guid);
    skr::Path resourcePath{ headerPath };
    resourcePath.replace_extension(u8".bin");
    auto resourceUri = resourcePath.string();
    FillRequest(request, header, vfs, resourceUri.c_str());
    request->OnRequestFileFinished();
    return true;
}

void LocalResourceRegistry::CancelRequestFile(ResourceRequest* requst)
{
}
} // namespace skr