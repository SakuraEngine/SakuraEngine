#pragma once
#include "SkrDAScript/env.hpp"

namespace skr {
namespace das {

struct LibraryDescriptor
{
    uint32_t init_mod_counts = 0;
    skr::das::Module* const* init_mods = nullptr;
    bool init_builtin_mods = false;
};

struct SKR_DASCRIPT_API Library
{
    static Library* Create(const LibraryDescriptor& desc) SKR_NOEXCEPT;
    static void Free(Library* library) SKR_NOEXCEPT;

    virtual ~Library() SKR_NOEXCEPT;
    virtual void add_module(Module* mod) SKR_NOEXCEPT = 0;
    virtual void add_builtin_module() SKR_NOEXCEPT = 0;
};

} // namespace das
} // namespace skr