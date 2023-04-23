#include "platform/vfs.h"
#include "platform/memory.h"
#include "utils/log.h"
#include "platform/filesystem.hpp"
#import <Foundation/Foundation.h>

inline static char8_t* duplicate_string(const char8_t* src_string) SKR_NOEXCEPT
{
    if (src_string != nullptr)
    {
        const size_t source_len = strlen((const char*)src_string);
        char8_t* result = (char8_t*)sakura_malloc(sizeof(char8_t) * (1 + source_len));
        strcpy((char*)result, (const char*)src_string);
        return result;
    }
    return nullptr;
}

skr_vfs_t* skr_create_vfs(const skr_vfs_desc_t* desc) SKR_NOEXCEPT
{
    bool success = true;
    auto fs = (skr_vfs_t*)sakura_calloc(1, sizeof(skr_vfs_t));
    fs->mount_type = desc->mount_type;
    skr_vfs_get_native_procs(&fs->procs);
    NSError* error = nil;

    NSFileManager* fileManager = [NSFileManager defaultManager];
    // override url
    if (desc->override_mount_dir)
    {
        if (fs->mount_dir) sakura_free(fs->mount_dir);
        fs->mount_dir = duplicate_string(desc->override_mount_dir);
    }
    // get application directory
    else if (desc->mount_type == SKR_MOUNT_TYPE_CONTENT)
    {
        fs->mount_dir = duplicate_string((const char8_t*)[[[[[NSBundle mainBundle] resourceURL] absoluteURL] path] UTF8String]);
        if (!fs->mount_dir)
            [fileManager changeCurrentDirectoryPath:[[NSBundle mainBundle] bundlePath]];
    }
    // save url
    else if (desc->mount_type == SKR_MOUNT_TYPE_DOCUMENTS)
    {
        auto documentUrl = [fileManager URLForDirectory:NSDocumentDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:true error:&error];
        if (!error)
        {
            fs->mount_dir = duplicate_string((const char8_t*)[[[documentUrl absoluteURL] path] UTF8String]);
        }
        else
        {
            SKR_LOG_ERROR("Error retrieving user documents directory: %s", [[error description] UTF8String]);
            success = false;
        }
    }
    // debug url
    else if (desc->mount_type == SKR_MOUNT_TYPE_DEBUG || desc->mount_type == SKR_MOUNT_TYPE_SAVE_0)
    {
#ifdef TARGET_IOS
        // Place log files in the application support directory on iOS.
        auto debugUrl = [fileManager URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:true error:&error];
        if (!error)
        {
            fs->mount_dir = duplicate_string(
            [[[gDebugUrl absoluteURL] path] UTF8String]);
        }
        else
        {
            SKR_LOG_ERROR("Error retrieving application support directory: %s", [[error description] UTF8String]);
        }
#else
        const char8_t* path = (const char8_t*)[[[NSBundle mainBundle] bundlePath] UTF8String];
        const skr::filesystem::path p(path);
        const auto ppstr = p.parent_path().u8string();
        fs->mount_dir = duplicate_string((const char8_t*)ppstr.c_str());
#endif
    }
    else if (desc->mount_type == SKR_MOUNT_TYPE_ABSOLUTE)
    {
        const char8_t* path = (const char8_t*)[[[NSBundle mainBundle] bundlePath] UTF8String];
        const skr::filesystem::path p(path);
        const auto pstr = p.u8string();
        fs->mount_dir = duplicate_string((const char8_t*)pstr.c_str());
    }

    success &= fs->mount_dir or desc->mount_type == SKR_MOUNT_TYPE_ABSOLUTE;
    if (!success) goto fatal;
    return fs;
fatal:
    skr_free_vfs(fs);
    return nullptr;
}

void skr_free_vfs(skr_vfs_t* fs) SKR_NOEXCEPT
{
    if (fs)
    {
        if (fs->mount_dir) sakura_free(fs->mount_dir);
        sakura_free(fs);
    }
}