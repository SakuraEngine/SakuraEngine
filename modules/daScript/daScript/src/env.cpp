#include "SkrDAScript/env.hpp"
#include "daScript/daScriptC.h"

namespace skr {
namespace das {

void Environment::Initialize(const EnvironmentDescriptor &desc) SKR_NOEXCEPT
{
    ::das_initialize();
}

void Environment::Finalize() SKR_NOEXCEPT
{
    ::das_shutdown();
}

} // namespace das
} // namespace skr