#include "SkrUsdTool/mesh_asset.hpp"
#include "UsdCore/mesh.hpp"
#include "UsdCore/stage.hpp"

namespace skd::asset
{
    void* SUSDMeshImporter::Import(skr_io_ram_service_t * io, SCookContext *context)
    {
        auto stage = USDCoreOpenStage(assetPath.c_str());
        if (!stage)
        {
            return nullptr;
        }
        auto prim = stage->GetPrimAtPath(primPath.c_str());
        cgltf_data* gltf = USDCoreConvertToGLTF(prim);
        return gltf;
    }

    void SUSDMeshImporter::Destroy(void * data)
    {
        cgltf_data* gltf = (cgltf_data*)data;
        cgltf_free(gltf);
    }
}