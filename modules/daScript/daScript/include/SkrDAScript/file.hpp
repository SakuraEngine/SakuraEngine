#pragma once
#include "SkrDAScript/env.hpp"

namespace skr {
namespace das {

struct FileAccessDescriptor
{
    uint32_t _nothing_ = 0;
};

struct SKR_DASCRIPT_API FileAccess
{
    static FileAccess* Create(const FileAccessDescriptor& desc) SKR_NOEXCEPT;
    static void Free(FileAccess* faccess) SKR_NOEXCEPT;

    virtual ~FileAccess() SKR_NOEXCEPT;
    virtual bool set_text_file(const char8_t* name, const char8_t* text, uint32_t len, bool own = false) SKR_NOEXCEPT = 0;
};


} // namespace das
} // namespace skr