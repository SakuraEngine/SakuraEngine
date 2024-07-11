#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrRT/io/ram_io.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrSerde/json/reader.h"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrTextureCompiler/texture_sampler_asset.hpp"

namespace skd
{
namespace asset
{

void* STextureSamplerImporter::Import(skr_io_ram_service_t* ioService, SCookContext* context)
{
    skr::BlobId blob        = nullptr;
    context->AddSourceFileAndLoad(ioService, jsonPath.c_str(), blob);
    SKR_DEFER({ blob.reset(); });
    /*
    const auto assetRecord = context->GetAssetRecord();
    {
        SKR_LOG_FMT_ERROR(u8"Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, ::error_message(doc.error()));
        return nullptr;
    }
    '*/
    skr::String jString(skr::StringView((const char8_t*)blob->get_data(), blob->get_size()));
    skr::archive::JsonReader jsonVal(jString.view());
    auto sampler_resource = SkrNew<skr_texture_sampler_resource_t>();
    skr::json::Read(&jsonVal, *sampler_resource);
    return sampler_resource;
}

void STextureSamplerImporter::Destroy(void* resource)
{
    auto sampler_resource = (skr_texture_sampler_resource_t*)resource;
    SkrDelete(sampler_resource);
}

bool STextureSamplerCooker::Cook(SCookContext* ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    //-----load config
    // no cook config for config, skipping

    //-----import resource object
    auto sampler_resource = ctx->Import<skr_texture_sampler_resource_t>();
    if (!sampler_resource) return false;
    SKR_DEFER({ ctx->Destroy(sampler_resource); });

    // write runtime resource to disk
    ctx->Save(*sampler_resource);
    return true;
}

} // namespace asset
} // namespace skd