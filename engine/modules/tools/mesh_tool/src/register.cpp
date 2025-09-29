#include "SkrCore/module/module.hpp"
#include "SkrMeshTool/mesh_asset.hpp"
#include "SkrRenderer/resources/mesh_resource.h"

struct MeshToolModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override
    {
#define _DEFAULT_COOKER(__COOKER_TYPE, __RESOURCE_TYPE) skr::RegisterCooker<__COOKER_TYPE>(true, skr::TypeInfo<__COOKER_TYPE>::get_guid(), skr::TypeInfo<__RESOURCE_TYPE>::get_guid());
        _DEFAULT_COOKER(skr::MeshCooker, skr::MeshResource)
#undef _DEFAULT_COOKER

#define _IMPORTER(__TYPE) skr::RegisterImporter<__TYPE>(skr::TypeInfo<__TYPE>::get_guid());
        _IMPORTER(skr::GltfMeshImporter)
        _IMPORTER(skr::ProceduralMeshImporter)
#undef _IMPORTER
    }
    virtual void on_unload() override
    {
    }
};
IMPLEMENT_DYNAMIC_MODULE(MeshToolModule, SkrMeshTool);
