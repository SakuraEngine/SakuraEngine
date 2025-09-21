#include "SkrCore/platform/vfs.h"
#include "SkrCore/log.h"
#include <string.h>
#include <SkrOS/filesystem.hpp>
#include "SkrCore/memory/memory.h"
#include <sys/stat.h>
#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
#else
    #include <dirent.h>
    #include <unistd.h>
#endif

#include "SkrProfile/profile.h"

struct skr_vfile_stdio_t : public skr_vfile_t {
    FILE* fh;
    uint64_t offset;
    skr::String filePath;
};

// Helper function to resolve path
static skr::Path resolve_path(skr_vfs_t* fs, const char8_t* path)
{
    skr::Path p;
    skr::Path in_p(path);
    if (in_p.is_absolute())
    {
        p = in_p;
    }
    else
    {
        if (fs->mount_dir)
        {
            p = skr::Path{fs->mount_dir};
            p /= path;
        }
        else
        {
            p = skr::Path{path};
        }
    }
    return p;
}

skr_vfile_t* skr_stdio_fopen(skr_vfs_t* fs, const char8_t* path, ESkrFileMode mode, ESkrFileCreation creation) SKR_NOEXCEPT
{
    skr::Path p;
    {
        SkrZoneScopedN("CalculatePath");
        p = resolve_path(fs, path);
    }
    auto filePath = p.string();
    const auto* filePathStr = filePath.data();
    
    // Create parent directories if needed when opening file for write/append
    if ((mode & (SKR_FM_WRITE | SKR_FM_APPEND)) && 
        (creation == SKR_FILE_CREATION_IF_NEEDED || creation == SKR_FILE_CREATION_ALWAYS_NEW))
    {
        auto parent_dir = p.parent_directory();
        if (!parent_dir.is_empty())
        {
            SkrZoneScopedN("CreateParentDirs");
            // Directory::create already handles the exists check internally and is thread-safe
            if (!skr::fs::Directory::create(parent_dir, true))
            {
                // Only log error if it's not because directory already exists
                if (!skr::fs::Directory::exists(parent_dir))
                {
                    SKR_LOG_ERROR(u8"Failed to create parent directories for file: %s", filePath.c_str());
                    return nullptr;
                }
            }
        }
    }
    
    const char8_t* modeStr = skr_vfs_filemode_to_string(mode);
    FILE* cfile = nullptr;
    {
        SkrZoneScopedN("stdio::fopen");
        SkrMessage((const char*)filePath.data(), filePath.size());
        cfile = fopen((const char*)filePathStr, (const char*)modeStr);
    }
    std::error_code ec = {};
    // SKR_LOG_TRACE(u8"CurrentPath: %s", skr::fs::current_path(ec).u8string().c_str());
    // Might fail to open the file for read+write if file doesn't exist
    if (!cfile)
    {
        if ((mode & SKR_FM_READ_WRITE) == SKR_FM_READ_WRITE)
        {
            SkrZoneScopedN("RetryOpenRW");

            modeStr = skr_vfs_overwrite_filemode_to_string(mode);
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
        return reinterpret_cast<skr_vfile_t*>(vfile);
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
        auto result = fwrite(out_buffer, 1, byte_count, vfile->fh);
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

// File system operations implementation
bool skr_stdio_fexists(skr_vfs_t* fs, const char8_t* path) SKR_NOEXCEPT
{
    auto p = resolve_path(fs, path);
    return skr::fs::File::exists(p);
}

bool skr_stdio_fis_directory(skr_vfs_t* fs, const char8_t* path) SKR_NOEXCEPT
{
    auto p = resolve_path(fs, path);
    return skr::fs::Directory::exists(p);
}

bool skr_stdio_fremove(skr_vfs_t* fs, const char8_t* path) SKR_NOEXCEPT
{
    auto p = resolve_path(fs, path);
    if (skr::fs::Directory::exists(p)) {
        return skr::fs::Directory::remove(p, false);
    } else {
        return skr::fs::File::remove(p);
    }
}

bool skr_stdio_frename(skr_vfs_t* fs, const char8_t* from, const char8_t* to) SKR_NOEXCEPT
{
    auto from_p = resolve_path(fs, from);
    auto to_p = resolve_path(fs, to);
    // TODO: implement move/rename functionality
    return false;
}

bool skr_stdio_fcopy(skr_vfs_t* fs, const char8_t* from, const char8_t* to) SKR_NOEXCEPT
{
    auto from_p = resolve_path(fs, from);
    auto to_p = resolve_path(fs, to);
    return skr::fs::File::copy(from_p, to_p, skr::fs::CopyOptions::OverwriteExisting);
}

int64_t skr_stdio_fmtime(skr_vfs_t* fs, const char8_t* path) SKR_NOEXCEPT
{
    auto p = resolve_path(fs, path);
    auto info = skr::fs::File::get_info(p);
    if (!info.exists()) return -1;
    
    // Convert FileTime to unix timestamp
    return skr::fs::filetime_to_unix(info.last_write_time);
}

// Directory operations implementation
bool skr_stdio_mkdir(skr_vfs_t* fs, const char8_t* path) SKR_NOEXCEPT
{
    auto p = resolve_path(fs, path);
    return skr::fs::Directory::create(p, true);
}

bool skr_stdio_rmdir(skr_vfs_t* fs, const char8_t* path) SKR_NOEXCEPT
{
    auto p = resolve_path(fs, path);
    return skr::fs::Directory::remove(p, true);
}

void skr_vfs_get_native_procs(struct skr_vfs_proctable_t* procs) SKR_NOEXCEPT
{
    procs->fopen = &skr_stdio_fopen;
    procs->fclose = &skr_stdio_fclose;
    procs->fread = &skr_stdio_fread;
    procs->fwrite = &skr_stdio_fwrite;
    procs->fsize = &skr_stdio_fsize;
    // File system operations
    procs->fexists = &skr_stdio_fexists;
    procs->fis_directory = &skr_stdio_fis_directory;
    procs->fremove = &skr_stdio_fremove;
    procs->frename = &skr_stdio_frename;
    procs->fcopy = &skr_stdio_fcopy;
    procs->fmtime = &skr_stdio_fmtime;
    // Directory operations
    procs->mkdir = &skr_stdio_mkdir;
    procs->rmdir = &skr_stdio_rmdir;
}