#pragma once
#include "SkrDAScript/env.hpp"

namespace skr {
namespace das {

struct LibraryDescriptor
{
    uint32_t _nothing_ = 0;
};

struct SKR_DASCRIPT_API Library
{
    static Library* Create(const LibraryDescriptor& desc) SKR_NOEXCEPT;
    static void Free(Library* library) SKR_NOEXCEPT;

    virtual ~Library() SKR_NOEXCEPT;
};

} // namespace das
} // namespace skr