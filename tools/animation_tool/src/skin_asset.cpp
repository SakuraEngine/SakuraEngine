#include "SkrAnimTool/skin_asset.h"
#include "SkrGLTFTool/gltf_utils.hpp"
#include "SkrAnim/resources/skin_resource.h"
#include "cgltf/cgltf.h"


namespace skd::asset
{
    bool SSkinCooker::Cook(SCookContext * ctx)
    {
        skr_skin_resource_t resource;
        // RawMesh* rawMesh = nullptr;
        // if(!CookMesh(ctx, resource.mesh, rawMesh))
        // {
        //     return false;
        // }
        // SKR_DEFER({ctx->Destroy(rawMesh);});

        SKR_UNIMPLEMENTED_FUNCTION();
        return false;
    }
}