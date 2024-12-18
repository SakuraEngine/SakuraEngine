#include "SkrCore/platform/vfs.h"
#include "SkrCore/log.h"
#include <string.h>
#include <SkrOS/filesystem.hpp>
#include "SkrCore/memory/memory.h"

#include "SkrProfile/profile.h"

struct skr_vfile_stdio_t : public skr_vfile_t {
    FILE* fh;
    uint64_t offset;
    decltype(skr::filesystem::path().u8string()) filePath;
};

skr_vfile_t* skr_stdio_fopen(skr_vfs_t* fs, const char8_t* path, ESkrFileMode mode, ESkrFileCreation creation) SKR_NOEXCEPT
{
    skr::filesystem::path p;
    {
        SkrZoneScopedN("CalculatePath");
        if(auto in_p = skr::filesystem::path(path); in_p.is_absolute())
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
    }
    auto filePath = p.u8string();
    const auto* filePathStr = filePath.c_str();
    const char8_t* modeStr = skr_vfs_filemode_to_string(mode);
    FILE* cfile = nullptr;
    {
        SkrZoneScopedN("stdio::fopen");
        SkrMessage((const char*)filePath.c_str(), filePath.size());
        cfile = fopen((const char*)filePathStr, (const char*)modeStr);
    }
    std::error_code ec = {};
    // SKR_LOG_TRACE(u8"CurrentPath: %s", skr::filesystem::current_path(ec).u8string().c_str());
    // Might fail to open the file for read+write if file doesn't exist
    if (!cfile)
    {
        if ((mode & SKR_FM_READ_WRITE) == SKR_FM_READ_WRITE)
        {
            SkrZoneScopedN("RetryOpenRW");

            modeStr = skr_vfs_overwirte_filemode_to_string(mode);
            cfile = fopen((const char*)filePath.c_str(), (const char*)modeStr);
        }
    }
    if (!cfile)
    {
        SKR_LOG_ERROR(u8"Error opening file: %s -- %s (error: %s)", filePath.c_str(), modeStr, strerror(errno));
        return nullptr;
    }
    {
        SkrZoneScopedN("WrapHandle");
        skr_vfile_stdio_t* vfile = SkrNew<skr_vfile_stdio_t>();
        vfile->mode = mode;
        vfile->fs = fs;
        vfile->fh = cfile;
        vfile->filePath = std::move(filePath);
        return vfile;
    }
}

size_t skr_stdio_fread(skr_vfile_t* file, void* out_buffer, size_t offset, size_t byte_count) SKR_NOEXCEPT
{
    if (file)
    {
        SkrZoneScopedN("vfs::fread");

        auto vfile = (skr_vfile_stdio_t*)file;
        if (vfile->offset != offset)
        {
            SkrZoneScopedN("stdio::fseek(offset)");
            fseek(vfile->fh, (long)offset, SEEK_SET); // seek to offset of file
            vfile->offset = offset;
        }
        size_t bytesRead = 0;
        {
            SkrZoneScopedN("stdio::fread");
            SkrMessage((const char*)vfile->filePath.c_str(), vfile->filePath.size());
            bytesRead = fread(out_buffer, 1, byte_count, vfile->fh);
            vfile->offset += bytesRead;
        }
        if (bytesRead != byte_count)
        {
            if (ferror(vfile->fh) != 0)
            {
                SKR_LOG_WARN(u8"Error reading from system FileStream: %s", strerror(errno));
            }
        }
        return bytesRead;
    }
    return -1;
}

size_t skr_stdio_fwrite(skr_vfile_t* file, const void* out_buffer, size_t offset, size_t byte_count) SKR_NOEXCEPT
{
    if (file)
    {
        auto vfile = (skr_vfile_stdio_t*)file;
        fseek(vfile->fh, (long)offset, SEEK_SET); // seek to offset of file
        auto result = fwrite(out_buffer, byte_count, 1, vfile->fh);
        fseek(vfile->fh, 0, SEEK_SET); // seek back to beginning of file
        return result;
    }
    return -1;
}

int64_t skr_stdio_fsize(const skr_vfile_t* file) SKR_NOEXCEPT
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

bool skr_stdio_fclose(skr_vfile_t* file) SKR_NOEXCEPT
{
    if (file)
    {
        SKR_ASSERT(file->fs->procs.fclose == &skr_stdio_fclose);
        auto vfile = (skr_vfile_stdio_t*)file;
        auto code = fclose(vfile->fh);
        SkrDelete(file);
        return code != EOF;
    }
    return false;
}

void skr_vfs_get_native_procs(struct skr_vfs_proctable_t* procs) SKR_NOEXCEPT
{
    procs->fopen = &skr_stdio_fopen;
    procs->fclose = &skr_stdio_fclose;
    procs->fread = &skr_stdio_fread;
    procs->fwrite = &skr_stdio_fwrite;
    procs->fsize = &skr_stdio_fsize;
}