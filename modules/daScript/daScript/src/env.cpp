#include "SkrDAScript/env.hpp"
#include "daScript/daScript.h"

namespace skr {
namespace das {
using namespace ::das;

void Environment::Initialize(const EnvironmentDescriptor &desc) SKR_NOEXCEPT
{
    NEED_ALL_DEFAULT_MODULES;
    ::das::Module::Initialize(); 
}

void Environment::Finalize() SKR_NOEXCEPT
{
    ::das::Module::Shutdown();
}

} // namespace das
} // namespace skr