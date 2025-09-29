#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"
#include "SkrShaderCompiler/shader_compiler.hpp"

namespace skr
{
void* ShaderOptionImporter::Import(skr::io::IRAMService* ioService, CookContext* context)
{
    skr::BlobId ioBuffer = {};
    context->AddSourceFileAndLoad(ioService, jsonPath.c_str(), ioBuffer);
    SKR_DEFER({ ioBuffer.reset(); });
    /*
    const auto assetMetaFile = context->GetAssetMetaFile();
    {
        SKR_LOG_FMT_ERROR(u8"Import shader options asset {} from {} failed, json parse error {}", assetMetaFile->guid, jsonPath, ::error_message(doc.error()));
        return nullptr;
    }
    '*/

    auto reader = skr::ArReadJson::ReadBuffer(ioBuffer->get_data(), ioBuffer->get_size());
    auto collection = SkrNew<ShaderOptionsResource>();
    reader.value(*collection);
    return collection;
}

void ShaderOptionImporter::Destroy(void* resource)
{
    auto options = (ShaderOptionsResource*)resource;
    SkrDelete(options);
}

bool ShaderOptionsCooker::Cook(CookContext* ctx)
{
    //-----load config
    // no cook config for config, skipping

    //-----import resource object
    auto options = ctx->Import<ShaderOptionsResource>();
    if (!options)
        return false;
    SKR_DEFER({ ctx->Destroy(options); });

    //------write resource object to disk
    ctx->Save(*options);
    return true;
}

uint32_t ShaderOptionsCooker::Version()
{
    return kDevelopmentVersion;
}

} // namespace skr