#pragma once
#include "SkrRenderer/resources/material_type_resource.hpp"

namespace skr
{
namespace resource
{

struct SMaterialTypeFactoryImpl : public SMaterialTypeFactory
{
    SMaterialTypeFactoryImpl(const SMaterialTypeFactory::Root& root)
        : root(root)
    {
    }

    skr_type_id_t GetResourceType() override
    {
        return skr::type::type_id<skr_material_type_resource_t>::get();
    }
    bool AsyncIO() override { return true; }
    bool Unload(skr_resource_record_t* record) override
    {
        // TODO: RC management for shader collection resource
        auto material_type = static_cast<skr_material_type_resource_t*>(record->resource);
        SkrDelete(material_type);
        return true;
    }
    ESkrInstallStatus Install(skr_resource_record_t* record) override
    {
        auto material_type = static_cast<skr_material_type_resource_t*>(record->resource);
        return material_type ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
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

SMaterialTypeFactory* SMaterialTypeFactory::Create(const Root &root)
{
    return SkrNew<SMaterialTypeFactoryImpl>(root);
}

void SMaterialTypeFactory::Destroy(SMaterialTypeFactory *factory)
{
    SkrDelete(factory);
}

}
}