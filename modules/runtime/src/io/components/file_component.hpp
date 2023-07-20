#pragma once
#include "SkrRT/io/io.h"
#include "SkrRT/platform/vfs.h"
#include "SkrRT/platform/dstorage.h"
#include "SkrRT/platform/guid.hpp"
#include "../components/component.hpp"

namespace skr {
namespace io {

template <>
struct IORequestComponentTID<struct IOFileComponent> 
{
    static constexpr skr_guid_t Get()
    {
        using namespace skr::guid::literals;
        return u8"6bf19e92-7180-42d5-9bb7-19cae4e8716d"_guid;
    } 
};
struct IOFileComponent : public IORequestComponent
{
    IOFileComponent(IIORequest* const request) SKR_NOEXCEPT 
        : IORequestComponent(request) 
    {
        
    }
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override { return IORequestComponentTID<IOFileComponent>::Get(); }
    
    uint64_t get_fsize() const SKR_NOEXCEPT
    {
        if (file)
        {
            SKR_ASSERT(!dfile);
            return skr_vfs_fsize(file);
        }
        else
        {
            SKR_ASSERT(dfile);
            SKR_ASSERT(!file);
            auto instance = skr_get_dstorage_instnace();
            SkrDStorageFileInfo info;
            skr_dstorage_query_file_info(instance, dfile, &info);
            return info.file_size;
        }
        return 0;
    }

    void set_vfs(skr_vfs_t* _vfs) SKR_NOEXCEPT { vfs = _vfs; }
    void set_path(const char8_t* p) SKR_NOEXCEPT { path = p; }
    const char8_t* get_path() const SKR_NOEXCEPT { return path.u8_str(); }

    skr::string path;
    skr_vfs_t* vfs = nullptr;
    skr_io_file_handle file = nullptr;
    SkrDStorageFileHandle dfile = nullptr;
};

} // namespace io
} // namespace skr