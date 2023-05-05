#include "SkrDAScript/env.hpp"
#include "daScript/daScript.h"

inline static void __das_initialize() { NEED_ALL_DEFAULT_MODULES; }

namespace skr {
namespace das {

Environment::~Environment() SKR_NOEXCEPT
{
    
}

void Environment::Initialize(const EnvironmentDescriptor &desc) SKR_NOEXCEPT
{
    ::__das_initialize();
    ::das::Module::Initialize(); 
}

void Environment::Finalize() SKR_NOEXCEPT
{
    ::das::Module::Shutdown();
}

} // namespace das
} // namespace skr