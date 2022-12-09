#include "SkrRenderer/resources/material_resource.hpp"

namespace skr
{
namespace resource
{

struct SMaterialFactoryImpl : public SMaterialFactory
{
    SMaterialFactoryImpl(const SMaterialFactoryImpl::Root& root)
        : root(root)
    {
    }

    skr_type_id_t GetResourceType() override
    {
        return skr::type::type_id<skr_material_resource_t>::get();
    }
    bool AsyncIO() override { return true; }
    bool Unload(skr_resource_record_t* record) override
    {
        // TODO: RC management for shader collection resource
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        SkrDelete(material);
        return true;
    }
    ESkrInstallStatus Install(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        return material ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
    }
    bool Uninstall(skr_resource_record_t* record) override
    {
        return true;
    }
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override
    {
        return SKR_INSTALL_STATUS_SUCCEED;
    }

    Root root;
};

SMaterialFactory* SMaterialFactory::Create(const Root &root)
{
    return SkrNew<SMaterialFactoryImpl>(root);
}

void SMaterialFactory::Destroy(SMaterialFactory *factory)
{
    SkrDelete(factory);
}

}
}