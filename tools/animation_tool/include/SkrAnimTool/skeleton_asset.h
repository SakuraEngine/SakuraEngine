#pragma once
#include "SkrAnimTool/module.configure.h"

#ifndef __meta__
#include "SkrAnimTool/skeleton_asset.genereated.h"
#endif
#include "SkrToolCore/asset/importer.hpp"
namespace skd::asset
{
    class SKR_ANIMTOOL_API SSkelGltfImporter : public skd::asset::SImporter
    {
        virtual ~SSkelGltfImporter() = default;
        virtual void* Import(skr::io::RAMService*, SCookContext* context) override;
        virtual void Destroy(void*) override;
        static uint32_t Version() { return kDevelopmentVersion; }
    };
}