#include <EASTL/array.h>
#include "utils/io.hpp"
#include "utils/log.hpp"
#include "utils/make_zeroed.hpp"

#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"
#include "SkrShaderCompiler/shader_compiler.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"

#include "tracy/Tracy.hpp"

namespace skd
{
namespace asset
{
void* SShaderOptionsImporter::Import(skr::io::RAMService* ioService, SCookContext* context)
{
    const auto assetRecord = context->GetAssetRecord();
    skr_async_ram_destination_t ioDestination = {};
    context->AddFileDependencyAndLoad(ioService, jsonPath.c_str(), ioDestination);
    SKR_DEFER({sakura_free(ioDestination.bytes);});

    auto jsonString = simdjson::padded_string((char8_t*)ioDestination.bytes, ioDestination.size);
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR("Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, simdjson::error_message(doc.error()));
        return nullptr;
    }
    auto json_value = doc.get_value().value_unsafe();

    // create source code wrapper
    auto collection = SkrNew<skr_shader_options_resource_t>();
    skr::json::Read(std::move(json_value), *collection);
    return collection;
}

void SShaderOptionsImporter::Destroy(void *resource)
{
    auto options = (skr_shader_options_resource_t*)resource;
    SkrDelete(options);
}

bool SShaderOptionsCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    //-----load config
    // no cook config for config, skipping
    //-----import resource object
    auto options = ctx->Import<skr_shader_options_resource_t>();
    if(!options) return false;
    SKR_DEFER({ ctx->Destroy(options); });
    //------write resource object
    eastl::vector<uint8_t> buffer;
    skr::binary::VectorWriter writer{&buffer};
    skr_binary_writer_t archive(writer);
    skr::binary::Archive(&archive, *options);
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

uint32_t SShaderOptionsCooker::Version()
{
    return kDevelopmentVersion;
}

} // namespace asset
} // namespace skd