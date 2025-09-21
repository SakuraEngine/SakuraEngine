#include "SkrCore/platform/vfs.h"
#include "SkrCore/log.h"
#include <SkrOS/filesystem.hpp>
#include "SkrCore/memory/memory.h"

struct skr_vfile_cfile_t : public skr_vfile_t {
    FILE* cfile;
};

skr_vfile_t* skr_unix_fopen(skr_vfs_t* fs, const char8_t* path,
ESkrFileMode mode, const char8_t* password, skr_vfile_t* out_file)
{
    skr::Path filePath{skr::String(fs->mount_dir ? fs->mount_dir : u8"")};
    filePath /= skr::String(path);
    auto filePathStr = filePath.string();
    const char8_t* modeStr = skr_vfs_filemode_to_string(mode);
    FILE* cfile = fopen(reinterpret_cast<const char*>(filePathStr.data()), (const char*)modeStr);
    std::error_code ec = {};
    auto current_dir = skr::fs::current_directory();
    SKR_LOG_TRACE(u8"CurrentPath: %s", current_dir.string().data());
    // Might fail to open the file for read+write if file doesn't exist
    if (!cfile)
    {
        if ((mode & SKR_FM_READ_WRITE) == SKR_FM_READ_WRITE)
        {
            modeStr = skr_vfs_overwrite_filemode_to_string(mode);
            cfile = fopen(filePathStr.c_str_raw(), (const char*)modeStr);
        }
    }
    if (!cfile)
    {
        SKR_LOG_ERROR(u8"Error opening file: %s -- %s (error: %s)",
        filePath.c_str(), modeStr, strerror(errno));
        return nullptr;
    }
    skr_vfile_cfile_t* vfile = SkrNew<skr_vfile_cfile_t>();
    vfile->mode = mode;
    vfile->fs = fs;
    vfile->cfile = cfile;
    return vfile;
}