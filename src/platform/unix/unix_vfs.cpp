#include "platform/vfs.h"
#include "utils/log.h"
#include <ghc/filesystem.hpp>

bool skr_unix_fopen(skr_vfs_t* fs, const char8_t* path,
ESkrFileMode mode, const char8_t* password, skr_vfile_t* out_file)
{
    ghc::filesystem::path filePath(fs->mount_dir ? fs->mount_dir : "");
    filePath /= path;
    const char8_t* modeStr = skr_vfs_filemode_to_string(mode);
    FILE* cfile = fopen(filePath.c_str(), modeStr);
    SKR_LOG_INFO("CurrentPath: %s", ghc::filesystem::current_path().c_str());
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
        return false;
    }
    out_file->mode = mode;
    out_file->fs = fs;
    out_file->cfile = cfile;
    return true;
}