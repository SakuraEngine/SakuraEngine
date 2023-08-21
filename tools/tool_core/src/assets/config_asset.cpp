#include "SkrRT/type/type.hpp"
#include "SkrRT/resource/config_resource.h"
#include "SkrRT/misc/log.hpp"
#include "SkrRT/misc/defer.hpp"
#include "SkrRT/io/ram_io.hpp"
#include "SkrToolCore/assets/config_asset.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/project/project.hpp"

namespace skd
{
namespace asset
{
void* SJsonConfigImporter::Import(skr_io_ram_service_t* ioService, SCookContext* context)
{
    const auto assetRecord = context->GetAssetRecord();
    auto type = skr_get_type(&configType);
    if (type == nullptr)
    {
        SKR_LOG_ERROR(u8"import resource %s failed, rtti is not load", assetRecord->path.u8string().c_str());
        return nullptr;
    }

    skr::BlobId blob = nullptr;
    context->AddFileDependencyAndLoad(ioService, assetPath.c_str(), blob);
    SKR_DEFER({blob.reset();});

    auto jsonString = simdjson::padded_string((char*)blob->get_data(), blob->get_size());
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR(u8"Import config asset {} from {} failed, json parse error {}", assetRecord->guid, assetPath, simdjson::error_message(doc.error()));
        return nullptr;
    }
    
    //skr::resource::SConfigFactory::NewConfig(configType);
    skr_config_resource_t* resource = SkrNew<skr_config_resource_t>();
    resource->SetType(configType);
    type->DeserializeText(resource->configData, doc.get_value().value_unsafe());
    return resource; //导入具体数据
}

void SJsonConfigImporter::Destroy(void* resource)
{
    SkrDelete((skr_config_resource_t*)resource);
    return;
}

uint32_t SConfigCooker::Version()
{
    return 0;
}

bool SConfigCooker::Cook(SCookContext* ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    // const auto assetRecord = ctx->GetAssetRecord();
    //-----load config
    // no cook config for config, skipping
    //-----import resource object
    auto resource = ctx->Import<skr_config_resource_t>();
    if(!resource)
        return false;
    SKR_DEFER({ ctx->Destroy(resource); });
    //-----emit dependencies
    // no static dependencies
    //-----cook resource
    // no cook needed for config, just binarize it
    //-----fetch runtime dependencies
    auto type = (skr::type::RecordType*)skr_get_type(&resource->configType);
    for (auto& field : type->GetFields())
    {
        if (field.type->Same(skr::type::type_of<skr_resource_handle_t>::get()))
        {
            auto handle = (skr_resource_handle_t*)((char*)resource->configData + field.offset);
            if (handle->is_null())
                continue;
            ctx->AddRuntimeDependency(handle->get_guid());
        }
    }
    ctx->Save(*resource);
    return true;
}
} // namespace asset
} // namespace skd