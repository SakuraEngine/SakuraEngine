#pragma once
#include "SkrGameAINavTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/asset/cooker.hpp"
#ifndef __meta__
#include "SkrGameAINavTool/navmesh_asset.generated.h" // IWYU pragma: export
#endif

namespace skd sreflect
{
namespace asset sreflect
{

sreflect_struct("guid": "37d07586-0901-480a-8dcd-1f1f8220569c")
sattr("serialize" : "json")
SKR_GAMEAI_NAVTOOL_API SNavmeshFromSceneImporter : public skd::asset::SImporter
{
    sattr("no-default" : true)
    skr_guid_t sceneGUID;
    virtual ~SNavmeshFromSceneImporter() = default;
    virtual void* Import(skr_io_ram_service_t*, SCookContext * context) override;
    virtual void Destroy(void*) override;
    static uint32_t Version() { return kDevelopmentVersion; }
}
sregister_importer();
}
}