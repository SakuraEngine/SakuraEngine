#pragma once
#include "SkrDAScript/annotation.hpp"

namespace skr {
namespace das {

struct SKR_DASCRIPT_API Module
{
    static Module* Create(const char8_t* name) SKR_NOEXCEPT;
    static void Free(Module* mod) SKR_NOEXCEPT;

    virtual ~Module() SKR_NOEXCEPT;
    virtual void add_annotation(Annotation* annotation) SKR_NOEXCEPT = 0;
};

} // namespace das
} // namespace skr