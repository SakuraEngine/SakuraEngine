#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrTextureCompiler/texture_sampler_asset.hpp"

namespace skd::asset
{

void* TextureSamplerImporter::Import(skr::io::IRAMService* ioService, CookContext* context)
{
    skr::BlobId blob = nullptr;
    context->AddSourceFileAndLoad(ioService, jsonPath.c_str(), blob);
    SKR_DEFER({ blob.reset(); });
    /*
    const auto assetMetaFile = context->GetAssetMetaFile();
    {
        SKR_LOG_FMT_ERROR(u8"Import shader options asset {} from {} failed, json parse error {}", assetMetaFile->guid, jsonPath, ::error_message(doc.error()));
        return nullptr;
    }
    '*/

    auto reader = skr::ArReadJson::ReadBuffer(blob->get_data(), blob->get_size());
    auto sampler_resource = SkrNew<TextureSamplerResource>();
    reader.value(*sampler_resource);
    return sampler_resource;
}

void TextureSamplerImporter::Destroy(void* resource)
{
    auto sampler_resource = (TextureSamplerResource*)resource;
    SkrDelete(sampler_resource);
}

bool TextureSamplerCooker::Cook(CookContext* ctx)
{
    //-----load config
    // no cook config for config, skipping

    //-----import resource object
    auto sampler_resource = ctx->Import<TextureSamplerResource>();
    if (!sampler_resource) return false;
    SKR_DEFER({ ctx->Destroy(sampler_resource); });

    // write runtime resource to disk
    ctx->Save(*sampler_resource);
    return true;
}

} // namespace skd::asset