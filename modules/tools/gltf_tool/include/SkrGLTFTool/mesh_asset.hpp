#pragma once
#include "SkrBase/config.h"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/asset/cooker.hpp"
#ifndef __meta__
    #include "SkrGLTFTool/mesh_asset.generated.h" // IWYU pragma: export
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "D72E2056-3C12-402A-A8B8-148CB8EAB922")
sattr("serialize" : "json")
GLTFTOOL_API SGltfMeshImporter final : public SImporter {
    sattr("no-default" : true)
    skr::String assetPath;

    skr::Vector<skr_guid_t> materials;

    bool invariant_vertices = false;
    bool install_to_ram     = false;
    bool install_to_vram    = true;

    void* Import(skr_io_ram_service_t*, SCookContext* context) override;
    void  Destroy(void* resource) override;
};

sreflect_struct("guid" : "5a378356-7bfa-461a-9f96-4bbbd2e95368")
GLTFTOOL_API SMeshCooker final : public SCooker {
    bool     Cook(SCookContext* ctx) override;
    uint32_t Version() override;
};
} // namespace asset sreflect
} // namespace skd sreflect