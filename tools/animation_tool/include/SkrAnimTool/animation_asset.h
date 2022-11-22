#pragma once
#include "SkrAnimTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#ifndef __meta__
#include "SkrAnimTool/animation_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "37d07586-0901-480a-8dcd-1f1f8220569c")
SKR_ANIMTOOL_API SAnimGltfImporter : public skd::asset::SImporter
{
    virtual ~SAnimGltfImporter() = default;
    virtual void* Import(skr::io::RAMService*, SCookContext* context) override;
    virtual void Destroy(void*) override;
    static uint32_t Version() { return kDevelopmentVersion; }
};
}
}