#pragma once
#include "SkrRT/io/io.h"
#include "SkrRT/platform/vfs.h"
#include "SkrRT/platform/dstorage.h"
#include "SkrRT/platform/guid.hpp"
#include "../components/component.hpp"

namespace skr {
namespace io {

template <>
struct IORequestComponentTID<struct PathSrcComponent> 
{
    static constexpr skr_guid_t Get();
};
struct PathSrcComponent : public IORequestComponent
{
    PathSrcComponent(IIORequest* const request) SKR_NOEXCEPT;
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override;
    
    void set_vfs(skr_vfs_t* _vfs) SKR_NOEXCEPT { vfs = _vfs; }
    void set_path(const char8_t* p) SKR_NOEXCEPT { path = p; }
    const char8_t* get_path() const SKR_NOEXCEPT { return path.u8_str(); }

    skr::string path;
    skr_vfs_t* vfs = nullptr;
};

template <>
struct IORequestComponentTID<struct MemorySrcComponent> 
{
    static constexpr skr_guid_t Get();
};
struct MemorySrcComponent final : public IORequestComponent
{
    MemorySrcComponent(IIORequest* const request) SKR_NOEXCEPT;
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override;

    void set_memory_src(uint8_t* memory, uint64_t bytes) SKR_NOEXCEPT
    {
        buffer = memory;
        size = bytes;
    }

    uint8_t* buffer = nullptr;
    uint64_t size = 0;
};

template <>
struct IORequestComponentTID<struct FileComponent> 
{
    static constexpr skr_guid_t Get();
};
struct FileComponent : public IORequestComponent
{
    FileComponent(IIORequest* const request) SKR_NOEXCEPT;
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override;
    
    uint64_t get_fsize() const SKR_NOEXCEPT;

    skr_io_file_handle file = nullptr;
    SkrDStorageFileHandle dfile = nullptr;
};

constexpr skr_guid_t IORequestComponentTID<struct PathSrcComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"6bf19e92-7180-42d5-9bb7-19cae4e8716d"_guid;
} 

constexpr skr_guid_t IORequestComponentTID<struct MemorySrcComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"3fd925a5-8f53-427c-aa8b-c30c385d4cec"_guid;
} 

constexpr skr_guid_t IORequestComponentTID<struct FileComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"d91c35e3-30a6-4909-afaa-d0bd37bd7c2f"_guid;
} 

} // namespace io
} // namespace skr