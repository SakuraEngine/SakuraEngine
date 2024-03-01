#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/material_asset.hpp"

namespace skd
{
namespace asset
{

void* SMaterialTypeImporter::Import(skr_io_ram_service_t* ioService, SCookContext *context)
{
    const auto assetRecord = context->GetAssetRecord();
    skr::BlobId blob = nullptr;
    context->AddFileDependencyAndLoad(ioService, jsonPath.c_str(), blob);
    SKR_DEFER({blob.reset();});

    auto jsonString = simdjson::padded_string((char*)blob->get_data(), blob->get_size());
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR(u8"Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, simdjson::error_message(doc.error()));
        return nullptr;
    }
    auto json_value = doc.get_value().value_unsafe();

    // create source code wrapper
    auto type_asset = SkrNew<skr_material_type_asset_t>();
    skr::json::Read(std::move(json_value), *type_asset);
    
    SKR_ASSERT(!type_asset->vertex_type.isZero() && "Vertex type must be specified for material type!");

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
    //-----load config
    // no cook config for config, skipping

    //-----import resource object
    auto material_type = ctx->Import<skr_material_type_asset_t>();
    if(!material_type) return false;
    SKR_DEFER({ ctx->Destroy(material_type); });

    // convert to runtime resource
    skr_material_type_resource_t runtime_material_type;
    runtime_material_type.version = material_type->version;
    runtime_material_type.passes.reserve(material_type->passes.size());
    for (const auto& pass : material_type->passes)
    for (const auto& shader_asset : pass.shader_resources)
    {
        ctx->AddStaticDependency(shader_asset.get_guid(), true);
        ctx->AddRuntimeDependency(shader_asset.get_guid());
        // simly write guids to runtime resource handle sequence
        runtime_material_type.passes.add(pass);
    }
    runtime_material_type.default_values.reserve(material_type->properties.size());
    for (const auto& property : material_type->properties)
    {
        skr_material_value_t runtime_value;
        runtime_value.slot_name = property.name;
        runtime_value.prop_type = property.prop_type;
        runtime_value.value = property.default_value;
        runtime_value.resource = property.default_resource.get_guid();
        runtime_material_type.default_values.add(runtime_value);
    }
    runtime_material_type.switch_defaults = material_type->switch_defaults;
    runtime_material_type.option_defaults = material_type->option_defaults;
    runtime_material_type.vertex_type = material_type->vertex_type;

    // write runtime resource to disk
    ctx->Save(runtime_material_type);
    return true;
}


} // namespace asset
} // namespace skd