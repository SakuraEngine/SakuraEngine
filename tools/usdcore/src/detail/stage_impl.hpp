#pragma once
#include "helpers.hpp"
#include "UsdCore/stage.hpp"
#include "pxr/usd/usd/stage.h"

namespace skd
{
struct SUSDStageImpl : public USDWrapperWithRC<SUSDStage>
{
    SUSDStageImpl(pxr::UsdStageRefPtr stage);
    ~SUSDStageImpl();

    virtual SUSDPrimId GetPseudoRoot() override;
    virtual SUSDPrimId GetDefaultPrim() override;
    virtual eastl::vector<SUSDPrimId> GetPrototypes() override;
    virtual SUSDPrimId GetPrimAtPath(const char* path) override;

protected:
    pxr::UsdStageRefPtr stage;
};
}