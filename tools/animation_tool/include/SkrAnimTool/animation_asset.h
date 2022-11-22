#pragma once
#include "SkrAnimTool/module.configure.h"

#ifndef __meta__
#include "SkrAnimTool/animation_asset.genereated.h"
#endif
#include "SkrToolCore/asset/importer.hpp"
namespace skd sreflect
{
namespace asset sreflect
{
    class SKR_ANIMTOOL_API SAnimGltfImporter : public skd::asset::SImporter
    {
        virtual ~SAnimGltfImporter() = default;
        virtual void* Import(skr::io::RAMService*, SCookContext* context) override;
        virtual void Destroy(void*) override;
        static uint32_t Version() { return kDevelopmentVersion; }
    };
}
}