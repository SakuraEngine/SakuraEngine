#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrShaderCompiler/assets/material_asset.hpp"
#include "utils/io.h"
#include "SkrToolCore/project/project.hpp"
#include "json/reader.h"

namespace skd
{
namespace asset
{

void* SMaterialTypeImporter::Import(skr_io_ram_service_t* ioService, SCookContext *context)
{
    const auto assetRecord = context->GetAssetRecord();
    skr_async_ram_destination_t destination = {};
    context->AddFileDependencyAndLoad(ioService, jsonPath.c_str(), destination);
    SKR_DEFER({sakura_free(destination.bytes);});

    auto jsonString = simdjson::padded_string((char8_t*)destination.bytes, destination.size);
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR("Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, simdjson::error_message(doc.error()));
        return nullptr;
    }
    auto json_value = doc.get_value().value_unsafe();

    // create source code wrapper
    auto type_asset = SkrNew<skr_material_type_asset_t>();
    skr::json::Read(std::move(json_value), *type_asset);
    return type_asset;
}

void SMaterialTypeImporter::Destroy(void *resource)
{
    auto type_asset = (skr_material_type_asset_t*)resource;
    SkrDelete(type_asset);
}

bool SMaterialTypeCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    //-----load config
    // no cook config for config, skipping

    //-----import resource object
    auto material_type = ctx->Import<skr_material_type_asset_t>();
    if(!material_type) return false;
    SKR_DEFER({ ctx->Destroy(material_type); });

    // convert to runtime resource
    skr_material_type_resource_t runtime_material_type;
    runtime_material_type.version = material_type->version;
    runtime_material_type.shader_resources.reserve(material_type->shader_assets.size());
    for (const auto& shader_asset : material_type->shader_assets)
    {
        ctx->AddRuntimeDependency(shader_asset.get_guid());
        // simly write guids to runtime resource handle sequence
        runtime_material_type.shader_resources.emplace_back(shader_asset.get_guid());
    }
    runtime_material_type.default_values.reserve(material_type->properties.size());
    for (const auto& property : material_type->properties)
    {
        skr_material_value_t runtime_value;
        runtime_value.slot_name = property.name;
        runtime_value.prop_type = property.prop_type;
        runtime_value.value = property.default_value;
        runtime_value.resource = property.default_resource.get_guid();
        runtime_material_type.default_values.emplace_back(runtime_value);
    }
    runtime_material_type.switch_defaults = material_type->switch_defaults;
    runtime_material_type.option_defaults = material_type->option_defaults;

    // write runtime resource to disk
    ctx->Save(runtime_material_type);
    return true;
}


} // namespace asset
} // namespace skd