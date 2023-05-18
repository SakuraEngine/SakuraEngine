#pragma once
#include "SkrAnimTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#ifndef __meta__
    #include "SkrAnimTool/skin_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{

sreflect_struct("guid" : "B863C921-3451-4024-A525-474D140099DB")
SKR_ANIMTOOL_API SSkinCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
}
sregister_default_cooker(u8"332C6133-7222-4B88-9B2F-E4336A46DF2C");
} // namespace asset sreflect
} // namespace skd sreflect