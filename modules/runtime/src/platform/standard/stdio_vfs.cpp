#include "platform/vfs.h"
#include "utils/log.h"
#include <string.h>
#include <ghc/filesystem.hpp>
#include "platform/memory.h"

struct skr_vfile_stdio_t : public skr_vfile_t {
    FILE* fh;
};

skr_vfile_t* skr_llfio_fopen(skr_vfs_t* fs, const char8_t* path, ESkrFileMode mode, ESkrFileCreation creation) SKR_NOEXCEPT
{
    ghc::filesystem::path p;
    if(auto in_p = ghc::filesystem::path(path); in_p.is_absolute())
    {
        p = in_p;
    } 
    else
    {
        p = fs->mount_dir ? fs->mount_dir : path;
        if(fs->mount_dir)
        {
            p /= path;
        }
    }
    auto filePath = p.u8string();
    const auto* filePathStr = filePath.c_str();
    const char8_t* modeStr = skr_vfs_filemode_to_string(mode);
    FILE* cfile = fopen(filePathStr, modeStr);
    SKR_LOG_INFO("CurrentPath: %s", ghc::filesystem::current_path().u8string().c_str());
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
        SKR_LOG_ERROR("Error opening file: %s -- %s (error: %s)", filePath.c_str(), modeStr, strerror(errno));
        return nullptr;
    }
    skr_vfile_stdio_t* vfile = SkrNew<skr_vfile_stdio_t>();
    vfile->mode = mode;
    vfile->fs = fs;
    vfile->fh = cfile;
    return vfile;
}

size_t skr_llfio_fread(skr_vfile_t* file, void* out_buffer, size_t offset, size_t byte_count) SKR_NOEXCEPT
{
    if (file)
    {
        auto vfile = (skr_vfile_stdio_t*)file;
        fseek(vfile->fh, offset, SEEK_SET); // seek to offset of file
        auto bytesRead = fread(out_buffer, byte_count, 1, vfile->fh);
        if (bytesRead != byte_count)
        {
            if (ferror(vfile->fh) != 0)
            {
                SKR_LOG_WARN("Error reading from system FileStream: %s", strerror(errno));
            }
        }
        fseek(vfile->fh, 0, SEEK_SET); // seek back to beginning of file
        return bytesRead;
    }
    return -1;
}

size_t skr_llfio_fwrite(skr_vfile_t* file, const void* out_buffer, size_t offset, size_t byte_count) SKR_NOEXCEPT
{
    if (file)
    {
        auto vfile = (skr_vfile_stdio_t*)file;
        fseek(vfile->fh, offset, SEEK_SET); // seek to offset of file
        auto result = fwrite(out_buffer, byte_count, 1, vfile->fh);
        fseek(vfile->fh, 0, SEEK_SET); // seek back to beginning of file
        return result;
    }
    return -1;
}

ssize_t skr_llfio_fsize(const skr_vfile_t* file) SKR_NOEXCEPT
{
    if (file)
    {
        auto vfile = (skr_vfile_stdio_t*)file;
        fseek(vfile->fh, 0, SEEK_END); // seek to end of file
        auto size = ftell(vfile->fh); // get current file pointer
        fseek(vfile->fh, 0, SEEK_SET); // seek back to beginning of file
        return size;
    }
    return -1;
}

bool skr_llfio_fclose(skr_vfile_t* file) SKR_NOEXCEPT
{
    if (file)
    {
        SKR_ASSERT(file->fs->procs.fclose == &skr_llfio_fclose);
        auto vfile = (skr_vfile_stdio_t*)file;
        auto code = fclose(vfile->fh);
        SkrDelete(file);
        return code != EOF;
    }
    return false;
}

void skr_vfs_get_native_procs(struct skr_vfs_proctable_t* procs) SKR_NOEXCEPT
{
    procs->fopen = &skr_llfio_fopen;
    procs->fclose = &skr_llfio_fclose;
    procs->fread = &skr_llfio_fread;
    procs->fwrite = &skr_llfio_fwrite;
    procs->fsize = &skr_llfio_fsize;
}