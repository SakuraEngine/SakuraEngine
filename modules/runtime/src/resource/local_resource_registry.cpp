﻿#include <platform/filesystem.hpp>
#include "SkrRT/misc/defer.hpp"
#include "SkrRT/platform/vfs.h"
#include "resource/local_resource_registry.hpp"
#include "resource/resource_header.hpp"
#include "SkrRT/misc/log.hpp"
#include "serde/binary/reader.h"
#include "SkrRT/platform/guid.hpp"

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
    skr::filesystem::path headerPath = skr::format(u8"{}.rh", guid).c_str();
    auto headerUri = headerPath.u8string();
    // TODO: 检查文件存在？
    SKR_LOG_BACKTRACE("Failed to find resource file: %s!", headerUri.c_str());
    auto file = skr_vfs_fopen(vfs, headerUri.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    if (!file) return false;
    SKR_DEFER({ skr_vfs_fclose(file); });
    uint32_t _fs_length = (uint32_t)skr_vfs_fsize(file);
    skr_resource_header_t header;
    if (_fs_length <= sizeof(skr_resource_header_t))
    {
        uint8_t buffer[sizeof(skr_resource_header_t)];
        if(skr_vfs_fread(file, buffer, 0, _fs_length) != _fs_length)
        {
            SKR_LOG_FMT_ERROR(u8"[SLocalResourceRegistry::RequestResourceFile] failed to read resource header! guid: {}", guid);
            return false;
        }
        skr::binary::SpanReader reader = { buffer, 0 };
        skr_binary_reader_t archive{reader};
        if(skr::binary::Read(&archive, header) != 0)
            return false;
    }
    else
    {
        uint8_t* buffer = (uint8_t*)sakura_malloc(_fs_length);
        SKR_DEFER({ sakura_free(buffer); });
        if(skr_vfs_fread(file, buffer, 0, _fs_length) != _fs_length)
        {
            SKR_LOG_FMT_ERROR(u8"[SLocalResourceRegistry::RequestResourceFile] failed to read resource header! guid: {}", guid);
            return false;
        }
        skr::binary::SpanReader reader = { {buffer, _fs_length}, 0 };
        skr_binary_reader_t archive{reader};
        if(skr::binary::Read(&archive, header) != 0)
            return false; 
    }
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