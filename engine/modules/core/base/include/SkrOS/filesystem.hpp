#pragma once
#include "SkrBase/config.h"
#include "SkrContainersDef/string.hpp"
#include "SkrContainersDef/span.hpp"
#include "SkrContainersDef/vector.hpp"
#include "SkrBase/containers/path/path.hpp"
#include <cstdint>

namespace skr::fs
{

// Timestamp type (100-nanosecond intervals since January 1, 1601 UTC)
// Compatible with Windows FILETIME and can represent all Unix timestamps
using FileTime = uint64_t;

// Convert between FileTime and Unix timestamp
inline FileTime unix_to_filetime(int64_t unix_time)
{
    // Unix epoch (1970-01-01) is 11644473600 seconds after Windows epoch (1601-01-01)
    return (unix_time + 11644473600LL) * 10000000LL;
}

inline int64_t filetime_to_unix(FileTime file_time)
{
    return (file_time / 10000000LL) - 11644473600LL;
}

// Error codes
enum class Error : uint32_t
{
    None = 0,
    NotFound,
    AccessDenied,
    AlreadyExists,
    NotADirectory,
    NotAFile,
    DirectoryNotEmpty,
    InvalidPath,
    TooManyOpenFiles,
    DiskFull,
    ReadOnlyFileSystem,
    NotSupported,
    Unknown
};

// File type enumeration
enum class FileType : uint8_t
{
    None = 0,
    Regular,
    Directory,
    Symlink,
    Block,
    Character,
    Fifo,
    Socket,
    Unknown
};

// File permissions (Unix-style, mapped on Windows)
enum class Permissions : uint16_t
{
    None = 0,

    // Owner permissions
    OwnerRead  = 0400,
    OwnerWrite = 0200,
    OwnerExec  = 0100,
    OwnerAll   = 0700,

    // Group permissions
    GroupRead  = 040,
    GroupWrite = 020,
    GroupExec  = 010,
    GroupAll   = 070,

    // Others permissions
    OthersRead  = 04,
    OthersWrite = 02,
    OthersExec  = 01,
    OthersAll   = 07,

    // Common combinations
    All       = 0777,
    ReadOnly  = 0444,
    ReadWrite = 0666,

    // Special flags
    SetUID = 04000,
    SetGID = 02000,
    Sticky = 01000,

    Unknown = 0xFFFF
};

// Copy options
enum class CopyOptions : uint32_t
{
    None              = 0,
    SkipExisting      = 1 << 0,
    OverwriteExisting = 1 << 1,
    UpdateExisting    = 1 << 2,
    Recursive         = 1 << 3,
    CopySymlinks      = 1 << 4,
    SkipSymlinks      = 1 << 5,
    DirectoriesOnly   = 1 << 6,
    CreateSymlinks    = 1 << 7,
    CreateHardLinks   = 1 << 8
};

// File open mode
enum class OpenMode : uint32_t
{
    Read      = 1 << 0,
    Write     = 1 << 1,
    Append    = 1 << 2,
    Truncate  = 1 << 3,
    Create    = 1 << 4,
    Exclusive = 1 << 5,
    Binary    = 1 << 6,

    // Common combinations
    ReadOnly  = Read,
    ReadWrite = Read | Write
};

// Seek origin
enum class SeekOrigin : uint8_t
{
    Begin   = 0,
    Current = 1,
    End     = 2
};

// Platform-specific file attributes
struct PlatformAttributes
{
#ifdef _WIN32
    uint32_t windows_attributes = 0; // FILE_ATTRIBUTE_*
#endif
#ifdef __APPLE__
    uint32_t macos_flags = 0;     // Mac-specific flags
    bool     is_bundle   = false; // .app, .framework, etc.
#endif
    uint32_t unix_mode = 0; // Unix permission bits
};

// File/Directory metadata
struct FileInfo
{
    Path        path;
    FileType    type        = FileType::None;
    uint64_t    size        = 0;
    Permissions permissions = Permissions::Unknown;

    // Timestamps
    FileTime creation_time    = 0;
    FileTime last_write_time  = 0;
    FileTime last_access_time = 0;

    // Platform-specific attributes
    PlatformAttributes platform_attrs;

    // Helpers
    bool is_directory() const { return type == FileType::Directory; }
    bool is_regular_file() const { return type == FileType::Regular; }
    bool is_symlink() const { return type == FileType::Symlink; }
    bool exists() const { return type != FileType::None; }
};

// Directory entry (for iteration)
struct DirectoryEntry
{
    Path     path;
    FileType type = FileType::None;

    // Get full file info (may require additional system call)
    FileInfo get_info() const;
};

// ============================================================================
// File Struct - Static methods for file operations
// ============================================================================

struct File
{
public:
    // Basic file operations
    static bool     exists(const Path& path);
    static bool     is_regular_file(const Path& path);
    static FileInfo get_info(const Path& path);
    static Error    get_last_error();

    // File manipulation
    static bool remove(const Path& path);
    static bool copy(const Path& from, const Path& to, CopyOptions options = CopyOptions::None);
    static bool move(const Path& from, const Path& to);
    static bool rename(const Path& from, const Path& to);

    // File size operations
    static uint64_t size(const Path& path);
    static bool     resize(const Path& path, uint64_t new_size);

    // Permissions and attributes
    static bool set_permissions(const Path& path, Permissions perms);
    static bool set_last_write_time(const Path& path, FileTime time);

    // Read/Write operations (convenience functions for small files)
    static bool read_all_bytes(const Path& path, skr::Vector<uint8_t>& out_data);
    static bool read_all_text(const Path& path, skr::String& out_text);
    static bool write_all_bytes(const Path& path, skr::Span<const uint8_t> data);
    static bool write_all_text(const Path& path, skr::StringView text);

    // Platform-specific
#ifdef __APPLE__
    static bool is_bundle(const Path& path);
    static Path get_bundle_resource_path(const Path& bundle_path);
#endif

public:
    // Instance methods for file handle operations
    File() = default;
    ~File();

    // Non-copyable, movable
    File(const File&)            = delete;
    File& operator=(const File&) = delete;
    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;

    // Open/Close
    bool open(const Path& path, OpenMode mode);
    void close();
    bool is_open() const;

    // Read/Write
    size_t read(void* buffer, size_t size);
    size_t write(const void* buffer, size_t size);

    // Line operations
    bool read_line(skr::String& out_line);
    bool write_line(skr::StringView line);

    // Seek/Tell
    bool    seek(int64_t offset, SeekOrigin origin);
    int64_t tell() const;
    int64_t get_size() const;

    // Flush
    bool flush();

    // Error handling
    Error get_error() const;

    // Platform handle access (for advanced use)
    void* native_handle() const { return handle_; }

private:
    void* handle_     = nullptr;
    Error last_error_ = Error::None;
};

// ============================================================================
// Directory Struct - Static methods for directory operations
// ============================================================================

struct Directory
{
public:
    // Basic directory operations
    static bool exists(const Path& path);
    static bool is_directory(const Path& path);
    static bool is_empty(const Path& path);

    // Directory manipulation
    static bool create(const Path& path, bool recursive = false);
    static bool remove(const Path& path, bool recursive = false);
    static bool copy(const Path& from, const Path& to, CopyOptions options = CopyOptions::Recursive);
    static bool move(const Path& from, const Path& to);

    // Directory listing
    static bool get_entries(const Path& path, skr::Vector<DirectoryEntry>& out_entries);
    static bool get_files(const Path& path, skr::Vector<Path>& out_files, bool recursive = false);
    static bool get_directories(const Path& path, skr::Vector<Path>& out_dirs, bool recursive = false);

    // Special directories
    static Path current();
    static bool set_current(const Path& path);
    static Path temp();
    static Path home();
    static Path executable(); // Get path to current executable

    // Platform-specific special directories
    static Path app_data();  // User app data directory
    static Path documents(); // User documents
    static Path desktop();   // User desktop
    static Path downloads(); // User downloads

#ifdef __APPLE__
    // macOS/iOS specific
    static Path application_support(); // ~/Library/Application Support
    static Path caches();              // ~/Library/Caches
    static Path bundle_path();         // Current app bundle path
#endif

#ifdef TARGET_IOS
    // iOS specific
    static Path documents_ios(); // App's Documents (synced)
    static Path library_ios();   // App's Library
    static Path tmp_ios();       // App's tmp
#endif
};

inline static auto current_directory() { return Directory::current(); }

// ============================================================================
// Directory Iterator
// ============================================================================

struct DirectoryIterator
{
public:
    // Iterator traits
    using value_type = DirectoryEntry;
    using reference  = const DirectoryEntry&;
    using pointer    = const DirectoryEntry*;

    // Constructors
    DirectoryIterator() = default;
    explicit DirectoryIterator(const Path& path);
    ~DirectoryIterator();

    // Non-copyable, movable
    DirectoryIterator(const DirectoryIterator&)            = delete;
    DirectoryIterator& operator=(const DirectoryIterator&) = delete;
    DirectoryIterator(DirectoryIterator&& other) noexcept;
    DirectoryIterator& operator=(DirectoryIterator&& other) noexcept;

    // Iterator operations
    DirectoryIterator& operator++();
    bool               operator==(const DirectoryIterator& other) const;
    bool               operator!=(const DirectoryIterator& other) const { return !(*this == other); }
    reference          operator*() const { return current_; }
    pointer            operator->() const { return &current_; }

    // Check if at end
    bool at_end() const;

    // Error handling
    Error get_error() const { return last_error_; }

private:
    void*          impl_ = nullptr; // Platform-specific implementation
    DirectoryEntry current_;
    Error          last_error_ = Error::None;

    void advance();
};

// Range-based for loop support
inline DirectoryIterator begin(DirectoryIterator iter) { return std::move(iter); }
inline DirectoryIterator end(const DirectoryIterator&) { return DirectoryIterator(); }

// ============================================================================
// Symbolic Link Struct - Static methods for symlink operations
// ============================================================================

struct Symlink
{
public:
    static bool exists(const Path& path);
    static bool create(const Path& link, const Path& target);
    static bool create_directory(const Path& link, const Path& target);
    static Path read(const Path& link);
    static bool remove(const Path& link);
};

// ============================================================================
// Path utilities
// ============================================================================

struct PathUtils
{
public:
    static Path absolute(const Path& p);
    static Path canonical(const Path& p); // Resolves symlinks, removes dots
    static Path relative(const Path& p, const Path& base = Directory::current());
    static Path proximate(const Path& p, const Path& base = Directory::current());

    // Path validation
    static bool is_valid(const Path& p);
    static bool has_invalid_chars(const Path& p);
};

// ============================================================================
// Filesystem monitoring (optional, platform-specific)
// ============================================================================

enum class WatchEvent : uint8_t
{
    Created          = 1 << 0,
    Deleted          = 1 << 1,
    Modified         = 1 << 2,
    Renamed          = 1 << 3,
    AttributeChanged = 1 << 4
};

using WatchCallback = void (*)(const Path& path, WatchEvent event, void* user_data);

struct FileSystemWatcher
{
public:
    FileSystemWatcher();
    ~FileSystemWatcher();

    // Non-copyable, non-movable
    FileSystemWatcher(const FileSystemWatcher&)            = delete;
    FileSystemWatcher& operator=(const FileSystemWatcher&) = delete;

    // Watch operations
    bool add_watch(const Path& path, WatchEvent events, WatchCallback callback, void* user_data = nullptr);
    bool remove_watch(const Path& path);
    void clear_all_watches();

    // Process events (call periodically or in separate thread)
    void process_events();

private:
    void* impl_; // Platform-specific implementation
};

} // namespace skr::fs