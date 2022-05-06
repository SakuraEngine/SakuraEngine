#include "platform/vfs.h"
#include "utils/log.h"
#include "llfio.hpp"
#include <ghc/filesystem.hpp>
#include "platform/memory.h"

namespace llfio = LLFIO_V2_NAMESPACE;
struct skr_vfile_llfio_t : public skr_vfile_t {
    llfio::file_handle fh;
};

inline static llfio::file_handle::mode skr_vfs_filemode_to_llfio_mode(ESkrFileMode mode)
{
    switch (mode)
    {
        case ESkrFileMode::SKR_FM_READ:
        case ESkrFileMode::SKR_FM_READ_BINARY:
            return llfio::file_handle::mode::read;
        case ESkrFileMode::SKR_FM_READ_APPEND:
        case ESkrFileMode::SKR_FM_APPEND:
        case ESkrFileMode::SKR_FM_APPEND_BINARY:
        case ESkrFileMode::SKR_FM_READ_APPEND_BINARY:
            return llfio::file_handle::mode::append;
        case ESkrFileMode::SKR_FM_WRITE:
        case ESkrFileMode::SKR_FM_WRITE_BINARY:
        case ESkrFileMode::SKR_FM_READ_WRITE:
        case ESkrFileMode::SKR_FM_READ_WRITE_BINARY:
        default:
            return llfio::file_handle::mode::write;
    }
}

inline static llfio::file_handle::creation skr_vfs_filecreation_to_llfio_creation(ESkrFileCreation creation)
{
    switch (creation)
    {
        case ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING:
            return llfio::handle::creation::open_existing;
        case ESkrFileCreation::SKR_FILE_CREATION_NOT_EXIST:
            return llfio::handle::creation::only_if_not_exist;
        case ESkrFileCreation::SKR_FILE_CREATION_IF_NEEDED:
            return llfio::handle::creation::if_needed;
        case ESkrFileCreation::SKR_FILE_CREATION_ALWAYS_NEW:
        default:
            return llfio::handle::creation::always_new;
    }
}

skr_vfile_t* skr_llfio_fopen(skr_vfs_t* fs, const char8_t* path,
ESkrFileMode mode, ESkrFileCreation creation)
{
    ghc::filesystem::path p(fs->mount_dir ? fs->mount_dir : path);
    if (fs->mount_dir)
        p /= path;
    skr_vfile_llfio_t* vfile = SkrNew<skr_vfile_llfio_t>();
    try
    {
        const auto llf_mode = skr_vfs_filemode_to_llfio_mode(mode);
        const auto llf_creation = skr_vfs_filecreation_to_llfio_creation(creation);
        // Open the file for read
        vfile->fh =
        llfio::file( //
        {},          // path_handle to base directory
        p.c_str(),   // path_view to path fragment relative to base directory
        llf_mode,    // default mode is read only
        llf_creation // default creation is open existing
                     // default caching is all
                     // default flags is none
        )
        .value(); // If failed, throw a filesystem_error exception
        vfile->fs = fs;
        vfile->mode = mode;
        vfile->size = vfile->fh.maximum_extent().value();
        return vfile;
    } catch (std::exception e)
    {
        SKR_LOG_WARN("filesystem error: failed to open file %s", p.c_str());
        SkrDelete(vfile);
        return nullptr;
    }
}

size_t skr_llfio_fread(skr_vfile_t* file, void* out_buffer, size_t offset, size_t byte_count)
{
    if (file)
    {
        try
        {
            auto vfile = (skr_vfile_llfio_t*)file;
            return llfio::read(
            vfile->fh, offset,                           // offset
            { { (llfio::byte*)out_buffer, byte_count } } // Single scatter buffer of the vector
                                                         // default deadline is infinite
            )
            .value();
        } catch (std::exception e)
        {
            SKR_LOG_WARN("filesystem error: failed to read file");
            return -1;
        }
    }
    return -1;
}

size_t skr_llfio_fwrite(skr_vfile_t* file, const void* out_buffer, size_t offset, size_t byte_count)
{
    if (file)
    {
        try
        {
            auto vfile = (skr_vfile_llfio_t*)file;
            return vfile->fh
            .write(offset, // offset
            {
            // gather list, buffers use std::byte
            { (const llfio::byte*)(out_buffer), byte_count } }
            // default deadline is infinite
            )
            .value(); // If failed, throw a filesystem_error exception
        } catch (std::exception e)
        {
            SKR_LOG_WARN("filesystem error: failed to write file");
            return -1;
        }
    }
    return -1;
}

ssize_t skr_llfio_fsize(const skr_vfile_t* file)
{
    if (file)
    {
        try
        {
            auto vfile = (skr_vfile_llfio_t*)file;
            return vfile->fh.maximum_extent().value();
        } catch (std::exception e)
        {
            SKR_LOG_WARN("filesystem error: failed to get file size");
            return -1;
        }
    }
    return -1;
}

bool skr_llfio_fclose(skr_vfile_t* file)
{
    if (file)
    {
        SKR_ASSERT(file->fs->procs.fclose == &skr_llfio_fclose);
        auto vfile = (skr_vfile_llfio_t*)file;
        auto result = llfio::close(vfile->fh);
        if (result.has_error())
        {
            SKR_LOG_WARN("filesystem error: close file failed: %s",
            result.error().message().c_str());
        }
        SkrDelete(file);
        return !result.has_error();
    }
    return false;
}

void skr_vfs_get_native_procs(struct skr_vfs_proctable_t* procs)
{
    procs->fopen = &skr_llfio_fopen;
    procs->fclose = &skr_llfio_fclose;
    procs->fread = &skr_llfio_fread;
    procs->fwrite = &skr_llfio_fwrite;
    procs->fsize = &skr_llfio_fsize;
}