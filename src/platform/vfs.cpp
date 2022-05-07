#include "platform/vfs.h"
#include <ghc/filesystem.hpp>
#include <EASTL/string.h>

using u8string = eastl::string;

void skr_vfs_get_parent_path(const char8_t* path, char8_t* output)
{
    const ghc::filesystem::path p(path);
    std::strcpy(output, p.parent_path().u8string().c_str());
}

void skr_vfs_append_path_component(const char8_t* path, const char8_t* component, char8_t* output)
{
    const ghc::filesystem::path p(path);
    const auto appended = p / component;
    std::strcpy(output, appended.u8string().c_str());
}

void skr_vfs_append_path_extension(const char8_t* path, const char8_t* extension, char8_t* output)
{
    u8string p(path);
    if (extension[0] != '.')
    {
        p.append(".");
    }
    const auto appended = p.append(extension);
    std::strcpy(output, appended.c_str());
}

skr_vfile_t* skr_vfs_fopen(skr_vfs_t* fs, const char8_t* path, ESkrFileMode mode, ESkrFileCreation creation)
{
    return fs->procs.fopen(fs, path, mode, creation);
}

size_t skr_vfs_fread(skr_vfile_t* file, void* out_buffer, size_t offset, size_t byte_count)
{
    return file->fs->procs.fread(file, out_buffer, offset, byte_count);
}

size_t skr_vfs_fwrite(skr_vfile_t* file, const void* in_buffer, size_t offset, size_t byte_count)
{
    return file->fs->procs.fwrite(file, in_buffer, offset, byte_count);
}

ssize_t skr_vfs_fsize(const skr_vfile_t* file)
{
    return file->fs->procs.fsize(file);
}

bool skr_vfs_fclose(skr_vfile_t* file)
{
    return file->fs->procs.fclose(file);
}