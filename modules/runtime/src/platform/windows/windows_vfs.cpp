#include "platform/vfs.h"
#include "platform/memory.h"
#include "utils/log.h"
#include "platform/filesystem.hpp"

#if !defined(XBOX)
    #include "shlobj.h"
    #include "commdlg.h"
    #include <WinBase.h>
#endif

inline static char8_t* duplicate_string(const char8_t* src_string) SKR_NOEXCEPT
{
    if (src_string != nullptr)
    {
        const size_t source_len = strlen((const char*)src_string);
        char8_t* result = (char8_t*)sakura_malloc(sizeof(char8_t) * (1 + source_len));
        strcpy_s((char*)result, source_len + 1, (const char*)src_string);
        return result;
    }
    return nullptr;
}

template <typename T>
static inline T withUTF16Path(const char* path, T (*function)(const wchar_t*)) SKR_NOEXCEPT
{
    size_t len = strlen(path);
    wchar_t* buffer = (wchar_t*)alloca((len + 1) * sizeof(wchar_t));

    size_t resultLength = MultiByteToWideChar(CP_UTF8, 0, path, (int)len, buffer, (int)len);
    buffer[resultLength] = 0;

    return function(buffer);
}

#define WIN_FS_MAX_PATH 256
skr_vfs_t* skr_create_vfs(const skr_vfs_desc_t* desc) SKR_NOEXCEPT
{
    SKR_ASSERT(desc);
    auto fs = (skr_vfs_t*)sakura_calloc(1, sizeof(skr_vfs_t));
    fs->mount_type = desc->mount_type;
    skr_vfs_get_native_procs(&fs->procs);
    fs->mount_dir = nullptr;

    // document dir
    // Override Resource mounts
    if (desc->override_mount_dir)
    {
        if (fs->mount_dir) sakura_free(fs->mount_dir);
        fs->mount_dir = duplicate_string(desc->override_mount_dir);
    }
    else if (desc->mount_type == SKR_MOUNT_TYPE_DOCUMENTS)
    {
        char documentPath[WIN_FS_MAX_PATH] = {};
        PWSTR userDocuments = NULL;
        SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &userDocuments);
        WideCharToMultiByte(CP_UTF8, 0, userDocuments, -1, documentPath, WIN_FS_MAX_PATH, NULL, NULL);
        CoTaskMemFree(userDocuments);
        fs->mount_dir = duplicate_string((const char8_t*)documentPath);
    }
    else
    {
        // Get application directory
        wchar_t utf16Path[WIN_FS_MAX_PATH];
        GetModuleFileNameW(0, utf16Path, WIN_FS_MAX_PATH);
        char applicationFilePath[WIN_FS_MAX_PATH] = {};
        WideCharToMultiByte(CP_UTF8, 0, utf16Path, -1, applicationFilePath, WIN_FS_MAX_PATH, NULL, NULL);
        const skr::filesystem::path p(applicationFilePath);
        const auto parentPath = p.parent_path().u8string();
        fs->mount_dir = duplicate_string(parentPath.c_str());
    }
    return fs;
}

void skr_free_vfs(skr_vfs_t* fs) SKR_NOEXCEPT
{
    if (fs)
    {
        if (fs->mount_dir) sakura_free(fs->mount_dir);
        sakura_free(fs);
    }
}