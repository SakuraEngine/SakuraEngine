#pragma once
#include "SkrGLTFTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/asset/cooker.hpp"
#ifndef __meta__
#include "SkrGLTFTool/mesh_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "D72E2056-3C12-402A-A8B8-148CB8EAB922")
sattr("serialize" : "json")
GLTFTOOL_API SGltfMeshImporter final : public SImporter
{
    sattr("no-default" : true)
    skr::string assetPath;
    
    bool invariant_vertices = false;
    bool install_to_ram = false;
    bool install_to_vram = true;

    void* Import(skr_io_ram_service_t*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sregister_importer();

sreflect_struct("guid" : "9A2C9CBF-517D-4197-BDE3-E40D85D88320")
sattr("serialize" : "json")
GLTFTOOL_API SMeshCookConfig
{
    sattr("no-default" : true)
    skr_guid_t vertexType;
};

sreflect_struct("guid" : "5a378356-7bfa-461a-9f96-4bbbd2e95368")
GLTFTOOL_API SMeshCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_default_cooker("3b8ca511-33d1-4db4-b805-00eea6a8d5e1");
} // namespace asset
} // namespace skd