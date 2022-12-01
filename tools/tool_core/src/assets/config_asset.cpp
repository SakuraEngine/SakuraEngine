#include "platform/configure.h"
#include "platform/memory.h"
#include "platform/vfs.h"
#include "containers/hashmap.hpp"
#include "resource/config_resource.h"
#include "json/reader.h"
#include <mutex>
#include "platform/debug.h"
#include "json/reader.h"
#include "type/type_registry.h"
#include "utils/log.hpp"
#include "utils/defer.hpp"
#include "utils/io.h"
#include "SkrToolCore/assets/config_asset.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/project/project.hpp"

namespace skd::asset
{
struct SConfigRegistryImpl : public SConfigRegistry
{
    void RegisterConfigType(skr_guid_t type, const SConfigTypeInfo& info) override
    {
        typeInfos.insert({type, info});
    }
    const SConfigTypeInfo* FindConfigType(skr_guid_t type) override
    {
        auto it = typeInfos.find(type);
        if (it != typeInfos.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    skr::flat_hash_map<skr_guid_t, SConfigTypeInfo, skr::guid::hash> typeInfos;
};

TOOL_CORE_API SConfigRegistry* GetConfigRegistry()
{
    static SConfigRegistryImpl registry;
    return &registry;
}
} // namespace skd::asset

namespace skd
{
namespace asset
{
void* SJsonConfigImporter::Import(skr_io_ram_service_t* ioService, SCookContext* context)
{
    auto registry = GetConfigRegistry();
    const auto assetRecord = context->GetAssetRecord();
    const auto typeInfo = registry->FindConfigType(configType);
    if (typeInfo == nullptr)
    {
        SKR_LOG_ERROR("import resource %s failed, type is not registered as config", assetRecord->path.u8string().c_str());
        return nullptr;
    }

    skr_async_ram_destination_t destination = {};
    context->AddFileDependencyAndLoad(ioService, assetPath.c_str(), destination);
    SKR_DEFER({sakura_free(destination.bytes);});

    auto jsonString = simdjson::padded_string((char8_t*)destination.bytes, destination.size);
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR("Import config asset {} from {} failed, json parse error {}", assetRecord->guid, assetPath, simdjson::error_message(doc.error()));
        return nullptr;
    }
    
    //skr::resource::SConfigFactory::NewConfig(configType);
    skr_config_resource_t* resource = SkrNew<skr_config_resource_t>();
    resource->SetType(configType);
    typeInfo->Import(doc.get_value().value_unsafe(), resource->configData);
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
    for (auto& field : type->fields)
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

bool SJsonConfigImporterFactory::CanImport(const SAssetRecord* record)
{
    if (record->path.extension() == ".json")
        return true;
    return false;
}

skr_guid_t SJsonConfigImporterFactory::GetResourceType()
{
    return get_type_id_skr_config_resource_t();
}

void SJsonConfigImporterFactory::CreateImporter(const SAssetRecord* record)
{
    // TODO: invoke user interface?
    SKR_UNIMPLEMENTED_FUNCTION();
}
} // namespace asset
} // namespace skd