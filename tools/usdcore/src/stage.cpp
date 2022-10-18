#include "containers/detail/sptr.hpp"
#include "helpers.hpp"
#include "UsdCore/stage.hpp"
#include "pxr/usd/usd/common.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/xform.h"

namespace skd
{
struct SUSDStageImpl : public USDWrapperWithRC<SUSDStage>
{
    SUSDStageImpl(pxr::UsdStageRefPtr stage) : stage(stage) {}
    ~SUSDStageImpl()
    {
        stage.Reset();
    }

protected:
    pxr::UsdStageRefPtr stage;
};

SUSDStageId USDCoreOpenStage(const char *path)
{
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(path);
    auto root = stage->GetPseudoRoot();
    return skr::SPtr<SUSDStageImpl>::Create(stage);
}

bool USDCoreSupportFile(const char* path)
{
    const bool supported = pxr::UsdStage::IsSupportedFile(path);
    return supported;
}
}