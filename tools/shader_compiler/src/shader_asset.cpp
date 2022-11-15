#include "SkrShaderCompiler/assets/shader_asset.h"
#include "SkrShaderCompiler/shader_compiler.h"
#include "skr_renderer/resources/shader_resource.hpp"
#include "utils/io.hpp"
#include "utils/log.hpp"

namespace skd
{
namespace asset
{
void* SShaderImporter::Import(skr::io::RAMService* ioService, SCookContext* context)
{
    auto path = context->AddFileDependency(assetPath.c_str());
    auto u8Path = path.u8string();

    const auto assetRecord = context->GetAssetRecord();
    // load file
    skr::task::event_t counter;
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request,void* data) noexcept {
        auto pCounter = (skr::task::event_t*)data;
        pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&counter;
    skr_async_io_request_t ioRequest = {};
    skr_async_ram_destination_t ioDestination = {};
    ioService->request(assetRecord->project->vfs, &ramIO, &ioRequest, &ioDestination);
    counter.wait(false);

    // create compiler
    const auto extention = path.extension().u8string();
    const auto sourceType = Util_GetShaderSourceTypeWithExtensionString(extention.c_str());
    auto compiler = SkrShaderCompiler_CreateByType(sourceType);
    return compiler;
}

void SShaderImporter::Destroy(void *resource)
{
    auto compiler = (IShaderCompiler*)resource;
    SkrDelete(compiler);
}

bool SShaderCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto uncompressed = ctx->Import<IShaderCompiler>();
    SKR_DEFER({ ctx->Destroy(uncompressed); });
    // compile & write bytecode to disk

    // make archive
    eastl::vector<uint8_t> resource_data;
    struct VectorWriter
    {
        eastl::vector<uint8_t>* buffer;
        int write(const void* data, size_t size)
        {
            buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
            return 0;
        }
    } writer{&resource_data};
    skr_binary_writer_t archive(writer);
    // write texture resource
    skr_platform_shader_resource_t resource;
    // TODO: REFACTOR THIS
    skr_platform_shader_identifier_t dxil_identifier = {};
    dxil_identifier.bytecode_type = CGPU_SHADER_BYTECODE_TYPE_DXIL;
    dxil_identifier.hash.flags = ~0;
    dxil_identifier.hash.encoded_digits[0] = ~0;
    dxil_identifier.hash.encoded_digits[1] = ~0;
    dxil_identifier.hash.encoded_digits[2] = ~0;
    dxil_identifier.hash.encoded_digits[3] = ~0;
    dxil_identifier.entry = "main";
    resource.identifilers.emplace_back(dxil_identifier);
    // format
    skr::binary::Write(&archive, resource);
    // write to file
    auto file = fopen(outputPath.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", 
            assetRecord->guid, assetRecord->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(resource_data.data(), resource_data.size(), 1, file);
    // write bytecode files
    {

    }
    return true;
}

uint32_t SShaderCooker::Version()
{
    return kDevelopmentVersion;
}

}
}