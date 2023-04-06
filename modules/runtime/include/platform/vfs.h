#pragma once
#include "platform/configure.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ESkrFileMode
{
    SKR_FM_READ = 1 << 0,
    SKR_FM_WRITE = 1 << 1,
    SKR_FM_APPEND = 1 << 2,
    SKR_FM_BINARY = 1 << 3,
    SKR_FM_ALLOW_READ = 1 << 4, // Read Access to Other Processes, Usefull for Log System
    SKR_FM_READ_WRITE = SKR_FM_READ | SKR_FM_WRITE,
    SKR_FM_READ_APPEND = SKR_FM_READ | SKR_FM_APPEND,
    SKR_FM_WRITE_BINARY = SKR_FM_WRITE | SKR_FM_BINARY,
    SKR_FM_READ_BINARY = SKR_FM_READ | SKR_FM_BINARY,
    SKR_FM_APPEND_BINARY = SKR_FM_APPEND | SKR_FM_BINARY,
    SKR_FM_READ_WRITE_BINARY = SKR_FM_READ | SKR_FM_WRITE | SKR_FM_BINARY,
    SKR_FM_READ_APPEND_BINARY = SKR_FM_READ | SKR_FM_APPEND | SKR_FM_BINARY,
    SKR_FM_WRITE_ALLOW_READ = SKR_FM_WRITE | SKR_FM_ALLOW_READ,
    SKR_FM_APPEND_ALLOW_READ = SKR_FM_READ | SKR_FM_ALLOW_READ,
    SKR_FM_READ_WRITE_ALLOW_READ = SKR_FM_READ | SKR_FM_WRITE | SKR_FM_ALLOW_READ,
    SKR_FM_READ_APPEND_ALLOW_READ = SKR_FM_READ | SKR_FM_APPEND | SKR_FM_ALLOW_READ,
    SKR_FM_WRITE_BINARY_ALLOW_READ = SKR_FM_WRITE | SKR_FM_BINARY | SKR_FM_ALLOW_READ,
    SKR_FM_APPEND_BINARY_ALLOW_READ = SKR_FM_APPEND | SKR_FM_BINARY | SKR_FM_ALLOW_READ,
    SKR_FM_READ_WRITE_BINARY_ALLOW_READ = SKR_FM_READ | SKR_FM_WRITE | SKR_FM_BINARY | SKR_FM_ALLOW_READ,
    SKR_FM_READ_APPEND_BINARY_ALLOW_READ = SKR_FM_READ | SKR_FM_APPEND | SKR_FM_BINARY | SKR_FM_ALLOW_READ
} ESkrFileMode;

typedef enum ESkrFileCreation
{
    SKR_FILE_CREATION_OPEN_EXISTING,
    SKR_FILE_CREATION_NOT_EXIST,
    SKR_FILE_CREATION_IF_NEEDED,
    SKR_FILE_CREATION_ALWAYS_NEW
} ESkrFileCreation;

typedef enum ESkrMountType
{
    /// Installed game directory / bundle resource directory
    SKR_MOUNT_TYPE_CONTENT = 0,
    /// For storing debug data such as log files. To be used only during development
    SKR_MOUNT_TYPE_DEBUG,
    /// Documents directory
    SKR_MOUNT_TYPE_DOCUMENTS,
#if defined(ANDROID)
    // System level files (/proc/ or equivalent if available)
    SKR_MOUNT_TYPE_SYSTEM,
#endif
    /// Save game data mount 0
    SKR_MOUNT_TYPE_SAVE_0,
    /// Empty mount for absolute paths
    SKR_MOUNT_TYPE_ABSOLUTE,
    SKR_MOUNT_TYPE_CUSTOM,
    SKR_MOUNT_TYPE_COUNT,
} ESkrMountType;

typedef struct skr_memory_stream_t {
    uint8_t* buffer;
    size_t cursor;
    size_t capacity;
    bool owner;
} skr_memory_stream_t;

typedef struct skr_vfile_t {
    struct skr_vfs_t* fs;
    struct skr_vfile_t* base; // for chaining streams
    ssize_t size;
    ESkrFileMode mode;
} skr_vfile_t;

typedef skr_vfile_t* (*SkrVFSProcFOpen)(struct skr_vfs_t* fs, const char8_t* path, ESkrFileMode mode, ESkrFileCreation creation);
typedef bool (*SkrVFSProcFClose)(skr_vfile_t* file);
typedef size_t (*SkrVFSProcFRead)(skr_vfile_t* file, void* out_buffer, size_t offset, size_t size_in_bytes);
typedef size_t (*SkrVFSProcFWrite)(skr_vfile_t* file, const void* in_buffer, size_t offset, size_t byte_count);
typedef ssize_t (*SkrVFSProcFSize)(const skr_vfile_t* file);
typedef bool (*SkrVFSProcFGetPropI64)(skr_vfile_t* file, int32_t prop, int64_t* out_value);
typedef bool (*SkrVFSProcFSetPropI64)(skr_vfile_t* file, int32_t prop, int64_t value);

typedef struct skr_vfs_proctable_t {
    SkrVFSProcFOpen fopen;
    SkrVFSProcFClose fclose;
    SkrVFSProcFRead fread;
    SkrVFSProcFWrite fwrite;
    SkrVFSProcFSize fsize;
    SkrVFSProcFGetPropI64 fget_prop_i64;
    SkrVFSProcFSetPropI64 fset_prop_i64;
} skr_vfs_proctable_t;

typedef struct skr_vfs_async_proctable_t {
    void* __useless_currently__;
} skr_vfs_async_proctable_t;

typedef struct skr_vfs_t {
    skr_vfs_proctable_t procs;
    skr_vfs_async_proctable_t async_procs;
    void* pUser;
    ESkrMountType mount_type;
    char8_t* mount_dir;
} skr_vfs_t;

typedef struct skr_vfs_desc_t {
    const char8_t* app_name;
    void* platform_data;
    ESkrMountType mount_type;
    const char8_t* override_mount_dir;
} skr_vfs_desc_t;

// file system
RUNTIME_API skr_vfs_t* skr_create_vfs(const skr_vfs_desc_t* desc) SKR_NOEXCEPT;
RUNTIME_API void skr_free_vfs(skr_vfs_t*) SKR_NOEXCEPT;

// file I/O
RUNTIME_API skr_vfile_t* skr_vfs_fopen(skr_vfs_t* fs, const char8_t* path, ESkrFileMode mode, ESkrFileCreation creation) SKR_NOEXCEPT;
RUNTIME_API size_t skr_vfs_fread(skr_vfile_t* file, void* out_buffer, size_t offset, size_t byte_count) SKR_NOEXCEPT;
RUNTIME_API size_t skr_vfs_fwrite(skr_vfile_t* file, const void* in_buffer, size_t offset, size_t byte_count) SKR_NOEXCEPT;
RUNTIME_API ssize_t skr_vfs_fsize(const skr_vfile_t* file) SKR_NOEXCEPT;
RUNTIME_API bool skr_vfs_fclose(skr_vfile_t* file) SKR_NOEXCEPT;

RUNTIME_API void skr_vfs_get_native_procs(struct skr_vfs_proctable_t* procs) SKR_NOEXCEPT;

static FORCEINLINE const char8_t* skr_vfs_filemode_to_string(ESkrFileMode mode)
{
    mode = (ESkrFileMode)(mode & ~SKR_FM_ALLOW_READ);
    switch (mode)
    {
        case SKR_FM_READ: return u8"r";
        case SKR_FM_WRITE: return u8"w";
        case SKR_FM_APPEND: return u8"a";
        case SKR_FM_READ_BINARY: return u8"rb";
        case SKR_FM_WRITE_BINARY: return u8"wb";
        case SKR_FM_APPEND_BINARY: return u8"ab";
        case SKR_FM_READ_WRITE: return u8"r+";
        case SKR_FM_READ_APPEND: return u8"a+";
        case SKR_FM_READ_WRITE_BINARY: return u8"rb+";
        case SKR_FM_READ_APPEND_BINARY: return u8"ab+";
        default: return u8"r";
    }
}
static FORCEINLINE const char8_t* skr_vfs_overwirte_filemode_to_string(ESkrFileMode mode)
{
    switch (mode)
    {
        case SKR_FM_READ_WRITE:
            return u8"w+";
        case SKR_FM_READ_WRITE_BINARY:
            return u8"wb+";
        default:
            return skr_vfs_filemode_to_string(mode);
    }
}
#ifdef __cplusplus
}
#endif
