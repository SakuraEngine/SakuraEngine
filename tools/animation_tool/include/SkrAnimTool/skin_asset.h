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
struct sreflect SKR_ANIMTOOL_API SSkinCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
}
sregister_cooker("332C6133-7222-4B88-9B2F-E4336A46DF2C");
} // namespace asset sreflect
} // namespace skd sreflect