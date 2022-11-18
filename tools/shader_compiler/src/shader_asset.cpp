#include <EASTL/array.h>
#include "utils/io.hpp"
#include "utils/log.hpp"
#include "utils/make_zeroed.hpp"

#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/shader_asset.h"
#include "SkrShaderCompiler/shader_compiler.h"
#include "SkrRenderer/resources/shader_resource.hpp"

#include "tracy/Tracy.hpp"

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
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request,void* data) noexcept {
        auto pCounter = (skr::task::event_t*)data;
        pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&counter;
    skr_async_request_t ioRequest = {};
    skr_async_ram_destination_t ioDestination = {};
    ioService->request(assetRecord->project->vfs, &ramIO, &ioRequest, &ioDestination);
    counter.wait(false);

    // create source code wrapper
    const auto extention = path.extension().u8string();
    const auto source_name = path.filename().replace_extension();
    const auto sourceType = Util_GetShaderSourceTypeWithExtensionString(extention.c_str());
    return SkrNew<ShaderSourceCode>(ioDestination.bytes, ioDestination.size, source_name.u8string().c_str(), sourceType);
}

void SShaderImporter::Destroy(void *resource)
{
    auto source = (ShaderSourceCode*)resource;
    SkrDelete(source);
}

bool SShaderCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto source_code = ctx->Import<ShaderSourceCode>();
    SKR_DEFER({ ctx->Destroy(source_code); });
    // Enumerate destination bytecode format
    // TODO: REFACTOR THIS
    eastl::vector<ECGPUShaderBytecodeType> byteCodeFormats = {
        ECGPUShaderBytecodeType::CGPU_SHADER_BYTECODE_TYPE_DXIL,
        ECGPUShaderBytecodeType::CGPU_SHADER_BYTECODE_TYPE_SPIRV
    };
    eastl::vector<skr_platform_shader_identifier_t> outIdentifiers;
    outIdentifiers.resize(byteCodeFormats.size());
    auto system = skd::asset::GetCookSystem();
    system->ParallelFor(byteCodeFormats.begin(), byteCodeFormats.end(), 1,
        [source_code, ctx, &byteCodeFormats, &outIdentifiers, outputPath]
        (const ECGPUShaderBytecodeType* pFormat, const ECGPUShaderBytecodeType* _) -> void
        {
            ZoneScopedN("DXC Compile Task");

            const ECGPUShaderBytecodeType format = *pFormat;
            const auto index = pFormat - byteCodeFormats.begin();
            auto compiler = SkrShaderCompiler_CreateByType(source_code->source_type);
            if (compiler->IsSupportedTargetFormat(format))
            {
                auto& identifier = outIdentifiers[index];
                // compile & write bytecode to disk
                const auto* shaderImporter = static_cast<SShaderImporter*>(ctx->GetImporter());
                auto compiled = compiler->Compile(format, *source_code, *shaderImporter);
                auto bytes = compiled->GetBytecode();
                auto hashed = compiled->GetHashCode(&identifier.hash.flags, identifier.hash.encoded_digits);
                if (hashed && !bytes.empty())
                {
                    // wirte bytecode to disk
                    const auto subdir = CGPUShaderBytecodeTypeNames[format];
                    auto basePath = outputPath.parent_path() / subdir;
                    const auto fname = skr::format("{}#{}-{}-{}-{}", 
                        identifier.hash.flags, identifier.hash.encoded_digits[0],
                        identifier.hash.encoded_digits[1], identifier.hash.encoded_digits[2], identifier.hash.encoded_digits[3]);
                    // create dir
                    std::error_code ec = {};
                    skr::filesystem::create_directories(basePath, ec);
                    // write bytes to file
                    {
                        auto bytesPath = basePath / (fname + ".bytes").c_str();
                        auto file = fopen(bytesPath.u8string().c_str(), "wb");
                        SKR_DEFER({ fclose(file); });
                        if (!file) SKR_UNREACHABLE_CODE();
                        fwrite(bytes.data(), bytes.size(), 1, file);
                    }
                    // write pdb to file
                    if (auto pdb = compiled->GetPDB();!pdb.empty())
                    {
                        auto pdbPath = basePath / (fname + ".pdb").c_str();
                        auto pdb_file = fopen(pdbPath.u8string().c_str(), "wb");
                        SKR_DEFER({ fclose(pdb_file); });
                        if (!pdb_file) SKR_UNREACHABLE_CODE();
                        fwrite(pdb.data(), pdb.size(), 1, pdb_file);
                    }
                }
                else
                {
                    SKR_UNREACHABLE_CODE();
                }
                compiler->FreeCompileResult(compiled);
                // fill platform identifier
                identifier.bytecode_type = format;
                identifier.entry = shaderImporter->entry;
            }
            SkrShaderCompiler_Destroy(compiler);
        });

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
    auto resource = make_zeroed<skr_platform_shader_resource_t>();
    resource.identifilers = outIdentifiers;
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

    return true;
}

uint32_t SShaderCooker::Version()
{
    return kDevelopmentVersion;
}

}
}