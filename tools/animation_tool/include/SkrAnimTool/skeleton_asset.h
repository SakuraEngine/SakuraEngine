#pragma once
#include "SkrAnimTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#ifndef __meta__
#include "SkrAnimTool/skeleton_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "1719ab02-7a48-45db-b101-949155f92cad")
SKR_ANIMTOOL_API SSkelGltfImporter : public skd::asset::SImporter
{
    virtual ~SSkelGltfImporter() = default;
    virtual void* Import(skr::io::RAMService*, SCookContext* context) override
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return nullptr;
    }
    virtual void Destroy(void*) override
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }
    static uint32_t Version() { return kDevelopmentVersion; }
};
} // namespace asset
} // namespace skd