#include "detail/stage_impl.hpp"
#include "detail/prim_impl.hpp"

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

SUSDStageId USDCoreOpenStage(const char *path)
{
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(path);
    auto root = stage->GetPseudoRoot();
    return skr::SObjectPtr<SUSDStageImpl>::Create(stage);
}

bool USDCoreSupportFile(const char* path)
{
    const bool supported = pxr::UsdStage::IsSupportedFile(path);
    return supported;
}
}