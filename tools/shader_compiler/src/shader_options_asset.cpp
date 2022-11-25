#include <EASTL/array.h>
#include "utils/io.hpp"
#include "utils/log.hpp"
#include "utils/make_zeroed.hpp"

#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"
#include "SkrShaderCompiler/shader_compiler.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"

#include "tracy/Tracy.hpp"

namespace skd
{
namespace asset
{
void* SShaderOptionsImporter::Import(skr::io::RAMService* ioService, SCookContext* context)
{
    auto path = context->AddFileDependency(jsonPath.c_str());
    auto u8Path = path.u8string();

    const auto assetRecord = context->GetAssetRecord();
    // load file
    skr::task::event_t counter;
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request,void* data) noexcept {
        auto pCounter = (skr::task::event_t*)data;
        pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&counter;
    skr_async_request_t ioRequest = {};
    skr_async_ram_destination_t ioDestination = {};
    ioService->request(assetRecord->project->vfs, &ramIO, &ioRequest, &ioDestination);
    counter.wait(false);
    auto jsonString = simdjson::padded_string((char8_t*)ioDestination.bytes, ioDestination.size);
    sakura_free(ioDestination.bytes);

    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR("Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, simdjson::error_message(doc.error()));
        return nullptr;
    }

    // create source code wrapper
    auto collectionType = skr::type::type_id<skr_shader_options_resource_t>::get();
    auto newType = skr_get_type(&collectionType);
    auto collection = static_cast<skr_shader_options_resource_t*>(newType->Malloc());
    newType->Construct(collection, nullptr, 0);
    newType->DeserializeText(collection, std::move(doc));
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
    struct VectorWriter
    {
        eastl::vector<uint8_t>* buffer;
        int write(const void* data, size_t size)
        {
            buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
            return 0;
        }
    } writer{&buffer};
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