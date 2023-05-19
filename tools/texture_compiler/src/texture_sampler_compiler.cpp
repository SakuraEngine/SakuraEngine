#include "SkrToolCore/asset/cook_system.hpp"
#include "misc/io.h"
#include "SkrToolCore/project/project.hpp"
#include "serde/json/reader.h"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrTextureCompiler/texture_sampler_asset.hpp"

namespace skd
{
namespace asset
{

void* STextureSamplerImporter::Import(skr_io_ram_service_t* ioService, SCookContext *context)
{
    const auto assetRecord = context->GetAssetRecord();
    skr_async_ram_destination_t destination = {};
    context->AddFileDependencyAndLoad(ioService, jsonPath.c_str(), destination);
    SKR_DEFER({sakura_free(destination.bytes);});

    auto jsonString = simdjson::padded_string((char*)destination.bytes, destination.size);
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR(u8"Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, simdjson::error_message(doc.error()));
        return nullptr;
    }
    auto json_value = doc.get_value().value_unsafe();

    // create source code wrapper
    auto sampler_resource = SkrNew<skr_texture_sampler_resource_t>();
    skr::json::Read(std::move(json_value), *sampler_resource);
    return sampler_resource;
}

void STextureSamplerImporter::Destroy(void *resource)
{
    auto sampler_resource = (skr_texture_sampler_resource_t*)resource;
    SkrDelete(sampler_resource);
}

bool STextureSamplerCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    //-----load config
    // no cook config for config, skipping

    //-----import resource object
    auto sampler_resource = ctx->Import<skr_texture_sampler_resource_t>();
    if(!sampler_resource) return false;
    SKR_DEFER({ ctx->Destroy(sampler_resource); });

    // write runtime resource to disk
    ctx->Save(*sampler_resource);
    return true;
}

} // namespace asset
} // namespace skd