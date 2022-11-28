#include "SkrShaderCompiler/assets/material_asset.hpp"
#include "utils/io.hpp"
#include "SkrToolCore/project/project.hpp"

namespace skd
{
namespace asset
{

void* SMaterialTypeImporter::Import(skr::io::RAMService* ioService, SCookContext *context)
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
    auto&& json_value = doc.get_value().value_unsafe();

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
    auto options = ctx->Import<skr_material_type_asset_t>();
    if(!options) return false;
    SKR_DEFER({ ctx->Destroy(options); });
    eastl::vector<uint8_t> buffer;
    buffer.emplace_back(1);

    //------save resource to disk
    auto file = fopen(outputPath.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SShaderOptionsCooker::Cook] failed to write cooked file for resource {}! path: {}", 
            assetRecord->guid, assetRecord->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, buffer.size(), file);
    return true;
}


} // namespace asset
} // namespace skd