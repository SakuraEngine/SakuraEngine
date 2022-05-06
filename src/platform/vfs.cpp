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
