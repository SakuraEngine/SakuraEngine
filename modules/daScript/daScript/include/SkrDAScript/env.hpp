#pragma once
#include "SkrDAScript/module.configure.h"
#include "platform/configure.h"

namespace skr {
namespace das {

struct EnvironmentDescriptor
{
    uint32_t _nothing_ = 0;
};

struct SKR_DASCRIPT_API Environment
{
    static void Initialize(const EnvironmentDescriptor& desc) SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;

    virtual ~Environment() SKR_NOEXCEPT;
    // virtual class ::das::Context* get_context() SKR_NOEXCEPT = 0;
};

} // namespace das
} // namespace skr