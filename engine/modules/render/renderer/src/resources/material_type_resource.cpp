#include "SkrRenderer/resources/material_type_resource.hpp"

namespace skr
{
using namespace skr;

struct MaterialTypeFactoryImpl : public MaterialTypeFactory
{
    MaterialTypeFactoryImpl(const MaterialTypeFactory::Root& root)
        : root(root)
    {
    }

    GUID GetResourceType() override
    {
        return ::skr::type_id_of<MaterialTypeResource>();
    }
    bool AsyncIO() override { return true; }
    bool Unload(SResourceRecord* record) override
    {
        // TODO: RC management for shader collection resource
        auto material_type = static_cast<MaterialTypeResource*>(record->resource);
        SkrDelete(material_type);
        return true;
    }
    ESkrInstallStatus Install(SResourceRecord* record) override
    {
        auto material_type = static_cast<MaterialTypeResource*>(record->resource);
        return material_type ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
    }
    bool Uninstall(SResourceRecord* record) override
    {
        return true;
    }
    ESkrInstallStatus UpdateInstall(SResourceRecord* record) override
    {
        return SKR_INSTALL_STATUS_SUCCEED;
    }

    Root root;
};

MaterialTypeFactory* MaterialTypeFactory::Create(const Root& root)
{
    return SkrNew<MaterialTypeFactoryImpl>(root);
}

void MaterialTypeFactory::Destroy(MaterialTypeFactory* factory)
{
    SkrDelete(factory);
}

} // namespace skr