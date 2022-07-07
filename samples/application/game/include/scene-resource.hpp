#pragma once
#include "GameRT/gamert.configure.h"
#include "resource/resource_factory.h"
namespace skg::resource
{
    using namespace skr::resource;
    struct GAMERT_API SSceneFactory : SResourceFactory
    {
        skr_type_id_t GetResourceType() override;

        ESkrLoadStatus Load(skr_resource_record_t* record) override;
        ESkrInstallStatus Install(skr_resource_record_t* record) override;
    };  
}