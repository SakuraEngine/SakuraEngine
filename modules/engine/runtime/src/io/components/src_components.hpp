#pragma once
#include "SkrGraphics/dstorage.h"
#include "SkrRT/io/io.h"
#include "SkrCore/platform/vfs.h"
#include "../components/component.hpp"

namespace skr {
namespace io {

template <>
struct CID<struct PathSrcComponent> 
{
    static constexpr skr_guid_t Get();
};
struct PathSrcComponent : public IORequestComponent
{
    PathSrcComponent(IIORequest* const request) SKR_NOEXCEPT;
    
    void set_vfs(skr_vfs_t* _vfs) SKR_NOEXCEPT { vfs = _vfs; }
    void set_path(const char8_t* p) SKR_NOEXCEPT { path = p; }
    skr_vfs_t* get_vfs() const SKR_NOEXCEPT { return vfs; }
    const char8_t* get_path() const SKR_NOEXCEPT { return path.is_empty() ? nullptr : path.c_str(); }
private:
    skr::String path;
    skr_vfs_t* vfs = nullptr;
};

template <>
struct CID<struct MemorySrcComponent> 
{
    static constexpr skr_guid_t Get();
};
struct MemorySrcComponent final : public IORequestComponent
{
    MemorySrcComponent(IIORequest* const request) SKR_NOEXCEPT;

    void set_memory_src(uint8_t* memory, uint64_t bytes) SKR_NOEXCEPT
    {
        data = memory;
        size = bytes;
    }

    uint8_t* data = nullptr;
    uint64_t size = 0;
};

template <>
struct CID<struct FileComponent> 
{
    static constexpr skr_guid_t Get();
};
struct FileComponent : public IORequestComponent
{
    FileComponent(IIORequest* const request) SKR_NOEXCEPT;
    
    uint64_t get_fsize() const SKR_NOEXCEPT;

    skr_io_file_handle file = nullptr;
    SkrDStorageFileHandle dfile = nullptr;
};

constexpr skr_guid_t CID<struct PathSrcComponent>::Get()
{
    using namespace skr::literals;
    return u8"6bf19e92-7180-42d5-9bb7-19cae4e8716d"_guid;
} 

constexpr skr_guid_t CID<struct MemorySrcComponent>::Get()
{
    using namespace skr::literals;
    return u8"3fd925a5-8f53-427c-aa8b-c30c385d4cec"_guid;
} 

constexpr skr_guid_t CID<struct FileComponent>::Get()
{
    using namespace skr::literals;
    return u8"d91c35e3-30a6-4909-afaa-d0bd37bd7c2f"_guid;
} 

} // namespace io
} // namespace skr