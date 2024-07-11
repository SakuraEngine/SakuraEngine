#include "SkrBase/misc/defer.hpp"
#include "SkrCore/log.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrRT/io/ram_io.hpp"
#include "SkrRT/resource/config_resource.h"
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
    auto type = skr::rttr::get_type_from_guid(configType);
    if (type == nullptr)
    {
        SKR_LOG_ERROR(u8"import resource %s failed, rtti is not load", assetRecord->path.u8string().c_str());
        return nullptr;
    }

    skr::BlobId blob = nullptr;
    context->AddSourceFileAndLoad(ioService, assetPath.c_str(), blob);
    SKR_DEFER({blob.reset();});
    /*
    const auto assetRecord = context->GetAssetRecord();
    {
        SKR_LOG_FMT_ERROR(u8"Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, ::error_message(doc.error()));
        return nullptr;
    }
    '*/
    skr::String jString(skr::StringView((const char8_t*)blob->get_data(), blob->get_size()));
    skr::archive::JsonReader jsonVal(jString.view());

    auto resource = SkrNew<skr_config_resource_t>();
    resource->SetType(configType);
    SKR_UNIMPLEMENTED_FUNCTION();
    // type->read_json(resource->configData, doc.get_value().value_unsafe());
    // skr::json::Read(&jsonVal, *resource);
    
    return resource;
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
    // TODO. resume it
    // auto type = (skr::type::RecordType*)skr_get_type(&resource->configType);
    // for (auto& field : type->GetFields())
    // {
    //     if (field.type->Same(skr::type::type_of<skr_resource_handle_t>::get()))
    //     {
    //         auto handle = (skr_resource_handle_t*)((char*)resource->configData + field.offset);
    //         if (handle->is_null())
    //             continue;
    //         ctx->AddRuntimeDependency(handle->get_guid());
    //     }
    // }
    ctx->Save(*resource);
    return true;
}
} // namespace asset
} // namespace skd