#pragma once
#include "UsdCore/module.configure.h"
#include "containers/sptr.hpp"

namespace skd
{
    struct SUSDStage : public skr::SInterface { virtual ~SUSDStage() = default; };
    using SUSDStageId = skr::SPtr<SUSDStage, false>;

    [[nodiscard("Perf: Opened USD stage must not be discard!")]]
    USDCORE_API SUSDStageId USDCoreOpenStage(const char* path);
    
    USDCORE_API bool USDCoreSupportFile(const char* path);
}