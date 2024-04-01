#pragma once
#include "SkrBase/config.h"
#include "SkrToolCore/asset/importer.hpp"
#ifndef __meta__
    #include "SkrAnimTool/skin_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{

sreflect_struct("guid" : "B863C921-3451-4024-A525-474D140099DB")
SKR_ANIMTOOL_API SSkinCooker final : public SCooker {
    bool     Cook(SCookContext* ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
};
} // namespace asset sreflect
} // namespace skd sreflect