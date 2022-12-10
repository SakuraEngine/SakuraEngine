#include "UsdCore/mesh.hpp"
#include "pxr/usd/usdGeom/mesh.h"
#include "detail/prim_impl.hpp"

namespace skd
{
    cgltf_data* USDCoreConvertToGLTF(const SUSDPrimId& primref)
    {
        auto prim = skr::reinterpret_pointer_cast<SUSDPrimImpl>(primref);
        pxr::UsdGeomMesh mesh(prim->prim);
        SKR_UNIMPLEMENTED_FUNCTION()
        return nullptr;
    }
}