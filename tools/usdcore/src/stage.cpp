#include "detail/stage_impl.hpp"
#include "detail/prim_impl.hpp"
#include "utils/log.h"

namespace skd
{
SUSDStageImpl::SUSDStageImpl(pxr::UsdStageRefPtr stage) 
    : stage(stage) 
{

}

SUSDStageImpl::~SUSDStageImpl()
{
    stage.Reset();
}

SUSDPrimId SUSDStageImpl::GetPseudoRoot()
{
    return skr::SObjectPtr<SUSDPrimImpl>::Create(stage->GetPseudoRoot());
}

SUSDPrimId SUSDStageImpl::GetDefaultPrim()
{
    return skr::SObjectPtr<SUSDPrimImpl>::Create(stage->GetDefaultPrim());
}

eastl::vector<SUSDPrimId> SUSDStageImpl::GetPrototypes()
{
    eastl::vector<SUSDPrimId> prototypes;
    for(auto& prim : stage->GetPrototypes())
    {
        prototypes.push_back(skr::SObjectPtr<SUSDPrimImpl>::Create(prim));
    }
    return prototypes;
}

SUSDPrimId SUSDStageImpl::GetPrimAtPath(const char* path)
{
    return skr::SObjectPtr<SUSDPrimImpl>::Create(stage->GetPrimAtPath(pxr::SdfPath(path)));
}

SUSDStageId USDCoreOpenStage(const char *path)
{
    ZoneScopedN("USDCoreOpenStage");
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(path);
    auto root = stage->GetPseudoRoot();
    for(auto& layer : stage->GetUsedLayers())
    {
        SKR_LOG_DEBUG("Layer: %s", layer->GetRealPath().c_str());
    }
    return skr::SObjectPtr<SUSDStageImpl>::Create(stage);
}

bool USDCoreSupportFile(const char* path)
{
    const bool supported = pxr::UsdStage::IsSupportedFile(path);
    return supported;
}
}