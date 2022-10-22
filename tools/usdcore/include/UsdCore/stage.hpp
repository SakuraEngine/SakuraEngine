#pragma once
#include "prim.hpp"

namespace skd
{
    struct SUSDStage : public skr::SInterface 
    { 
        virtual ~SUSDStage() = default; 
        virtual SUSDPrimId GetPseudoRoot() = 0;
        virtual SUSDPrimId GetDefaultPrim() = 0;
        // virtual SUSDPrimId GetPrimAtPath() = 0;
    };
    using SUSDStageId = skr::SObjectPtr<SUSDStage>;

    [[nodiscard("Perf: Opened USD stage must not be discard!")]]
    USDCORE_API SUSDStageId USDCoreOpenStage(const char* path);
    
    USDCORE_API bool USDCoreSupportFile(const char* path);
}