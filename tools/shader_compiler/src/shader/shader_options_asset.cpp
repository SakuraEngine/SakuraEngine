#include <EASTL/array.h>
#include "utils/io.h"
#include "utils/log.hpp"
#include "utils/make_zeroed.hpp"

#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"
#include "SkrShaderCompiler/shader_compiler.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "json/reader.h"

#include "tracy/Tracy.hpp"

namespace skd
{
namespace asset
{
void* SShaderOptionsImporter::Import(skr_io_ram_service_t* ioService, SCookContext* context)
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
    //-----load config
    // no cook config for config, skipping
    
    //-----import resource object
    auto options = ctx->Import<skr_shader_options_resource_t>();
    if(!options) return false;
    SKR_DEFER({ ctx->Destroy(options); });

    //------write resource object to disk
    ctx->Save(*options);
    return true;
}

uint32_t SShaderOptionsCooker::Version()
{
    return kDevelopmentVersion;
}

} // namespace asset
} // namespace skd