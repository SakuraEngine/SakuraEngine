#include "SkrGLTFTool/mesh_asset.hpp"
#include "SkrRenderer/resources/mesh_resource.h"

struct _GLTFToolRegister {
    _GLTFToolRegister()
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skd::asset::RegisterCooker<__COOKER_TYPE>(true, skr::rttr::RTTRTraits<__COOKER_TYPE>::get_guid(), skr::rttr::RTTRTraits<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skd::asset::SMeshCooker, skr::renderer::MeshResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skd::asset::RegisterImporter<__TYPE>(skr::rttr::RTTRTraits<__TYPE>::get_guid());
        _IMPORTER(skd::asset::SGltfMeshImporter)
#undef _IMPORTER
    }
} _gltf_tool_register;