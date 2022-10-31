#include "platform/vfs.h"
#include "utils/log.h"
#include <platform/filesystem.hpp>
#include "platform/memory.h"

struct skr_vfile_cfile_t : public skr_vfile_t {
    FILE* cfile;
};

skr_vfile_t* skr_unix_fopen(skr_vfs_t* fs, const char8_t* path,
ESkrFileMode mode, const char8_t* password, skr_vfile_t* out_file)
{
    skr::filesystem::path filePath(fs->mount_dir ? fs->mount_dir : "");
    filePath /= path;
    const char8_t* modeStr = skr_vfs_filemode_to_string(mode);
    FILE* cfile = fopen(filePath.c_str(), modeStr);
    std::error_code ec = {};
    SKR_LOG_INFO("CurrentPath: %s", skr::filesystem::current_path(ec).c_str());
    // Might fail to open the file for read+write if file doesn't exist
    if (!cfile)
    {
        if ((mode & SKR_FM_READ_WRITE) == SKR_FM_READ_WRITE)
        {
            modeStr = skr_vfs_overwirte_filemode_to_string(mode);
            cfile = fopen(filePath.c_str(), modeStr);
        }
    }
    if (!cfile)
    {
        SKR_LOG_ERROR("Error opening file: %s -- %s (error: %s)",
        filePath.c_str(), modeStr, strerror(errno));
        return nullptr;
    }
    skr_vfile_cfile_t* vfile = SkrNew<skr_vfile_cfile_t>();
    vfile->mode = mode;
    vfile->fs = fs;
    vfile->cfile = cfile;
    return vfile;
}