#include <EASTL/array.h>
#include "utils/io.hpp"
#include "utils/log.hpp"
#include "utils/make_zeroed.hpp"

#include "json/writer.h"

#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"
#include "SkrShaderCompiler/shader_compiler.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"

#include "utils/cartesian_product.hpp"

#include "tracy/Tracy.hpp"

namespace skd
{
namespace asset
{
ShaderSourceCode::~ShaderSourceCode() SKR_NOEXCEPT
{
    sakura_free(bytes);    
}

void* SShaderImporter::Import(skr::io::RAMService* ioService, SCookContext* context)
{
    auto path = context->AddFileDependency(sourcePath.c_str());
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

// [x: "on", y: "a", z: "1"]
using unique_option_variant_t = eastl::vector<skr_shader_option_instance_t>;
// [ [z: "on", y: "a", z: "1"], [x: "on", y: "a", z: "2"] ...]
using option_variant_seq_t = eastl::vector<unique_option_variant_t>;
using variant_seq_hashe_seq_t = eastl::vector<skr_stable_shader_hash_t>;
void cartesian_variants(skr::span<skr_shader_options_resource_t*> options, eastl::vector<skr_shader_option_t>& out_flatten_options,
    option_variant_seq_t& out_variants, variant_seq_hashe_seq_t& out_stable_hahses)
{
    // flat and well sorted
    // [ x: ["on", "off"], y: ["a", "b", "c"], z: ["1", "2"] ]
    skr_shader_options_resource_t::flatten_options(out_flatten_options, options);

    // [ ["on", "off"], ["a", "b", "c"], ["1", "2"] ]
    eastl::vector<eastl::vector<eastl::string>> selection_seqs = {};
    selection_seqs.resize(out_flatten_options.size());
    for (size_t i = 0u; i < out_flatten_options.size(); ++i)
    {
        selection_seqs[i] = out_flatten_options[i].value_selections;
    }

    // [ [z: "on", y: "a", z: "1"], [x: "on", y: "a", z: "2"] ...]
    (void)out_variants;
    if (!selection_seqs.empty())
    {
        // [ ["on", "a", "1"], ["on", "a", "2"] ...]
        skr::cartesian_product<eastl::string> cartesian(selection_seqs);
        while (cartesian.has_next())
        {
            eastl::vector<skr_shader_option_instance_t> option_seq = {};
            const auto sequence = cartesian.next();
            option_seq.resize(sequence.size());
            SKR_ASSERT(sequence.size() == out_flatten_options.size());
            for (size_t idx = 0u; idx < option_seq.size(); ++idx)
            {
                option_seq[idx].key = out_flatten_options[idx].key;
                option_seq[idx].value = sequence[idx];
            }
            out_variants.emplace_back(option_seq);
            const auto stable_hash = 
                skr_shader_option_instance_t::calculate_stable_hash({ option_seq.data(), option_seq.size() });
            out_stable_hahses.emplace_back(stable_hash);
        }
    }
    else
    {
        out_stable_hahses.emplace_back(0u, 0u, 0u, 0u); // emplace an zero hash
        out_variants.emplace_back(); // emplace an empty option sequence
    }
}

// skr_shader_options_resource_t:
// LEVEL["level0", "level1", "level2"]: 
//    same as "key": ["level0", "level1", "level2"] but def(level2) includes def(level1) & def(level0)) 
// SELECT["selection0", "selection1", "selection2"]: 
//    same as "key": ["selection0", "selection1", "selection2"]
// SWITCH["switch"]: 
//    same as "switch": ["on", "off"]
bool SShaderCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto source_code = ctx->Import<ShaderSourceCode>();
    SKR_DEFER({ ctx->Destroy(source_code); });
    // Calculate all macro combines (shader variants)
    eastl::vector<skr_shader_options_resource_t*> switch_assets = {};
    eastl::vector<skr_shader_options_resource_t*> option_assets = {};
    auto importer = static_cast<SShaderImporter*>(ctx->GetImporter());
    for (auto switch_asset : importer->switch_assets)
    {
        const auto guid = switch_asset.get_guid();
        auto idx = ctx->AddStaticDependency(guid, true);
        auto collection = static_cast<skr_shader_options_resource_t*>(ctx->GetStaticDependency(idx).get_ptr());
        switch_assets.emplace_back(collection);
    }
    for (auto option_asset : importer->option_assets)
    {
        const auto guid = option_asset.get_guid();
        auto idx = ctx->AddStaticDependency(guid, true);
        auto collection = static_cast<skr_shader_options_resource_t*>(ctx->GetStaticDependency(idx).get_ptr());
        option_assets.emplace_back(collection);
    }

    eastl::vector<skr_shader_option_t> flat_static_options = {};
    option_variant_seq_t static_variants = {};
    variant_seq_hashe_seq_t static_stable_hashes = {};
    cartesian_variants(switch_assets, flat_static_options, static_variants, static_stable_hashes);

    eastl::vector<skr_shader_option_t> flat_dynamic_options = {};
    option_variant_seq_t dynamic_variants = {};
    variant_seq_hashe_seq_t dynamic_stable_hashes = {};
    cartesian_variants(option_assets, flat_dynamic_options, dynamic_variants, dynamic_stable_hashes);

    // Enumerate destination bytecode format
    // TODO: REFACTOR THIS
    eastl::vector<ECGPUShaderBytecodeType> byteCodeFormats = {
        ECGPUShaderBytecodeType::CGPU_SHADER_BYTECODE_TYPE_DXIL,
        ECGPUShaderBytecodeType::CGPU_SHADER_BYTECODE_TYPE_SPIRV
    };
    // begin compile
    auto system = skd::asset::GetCookSystem();
    eastl::vector<skr_platform_shader_resource_t> allOutResources(static_variants.size());
    
    // foreach variants
    system->ParallelFor(static_variants.begin(), static_variants.end(), 1,
        [&] (const auto* pVariant, const auto* _) -> void
        {
            const uint64_t static_varidx = pVariant - static_variants.begin();
            auto& outResource = allOutResources[static_varidx];
            outResource.stable_hash = static_stable_hashes[static_varidx];
            for (const auto dyn_hash : dynamic_stable_hashes)
            {
                outResource.option_variants[dyn_hash] = {};
                outResource.option_variants[dyn_hash].resize(byteCodeFormats.size());
            }

    // foreach dynamic variants
    system->ParallelFor(dynamic_variants.begin(), dynamic_variants.end(), 1,
        [&] (const auto* pDynamicVariant, const auto* __) -> void
    {
        const uint64_t dynamic_varidx = pDynamicVariant - dynamic_variants.begin();
        const auto dyn_hash = dynamic_stable_hashes[dynamic_varidx];

    // foreach target profiles
    system->ParallelFor(byteCodeFormats.begin(), byteCodeFormats.end(), 1,
        [&] (const ECGPUShaderBytecodeType* pFormat, const ECGPUShaderBytecodeType* ___) -> void
        {
            ZoneScopedN("Shader Compile Task");

            const ECGPUShaderBytecodeType format = *pFormat;
            const uint64_t fmtIndex = pFormat - byteCodeFormats.begin();

            auto compiler = SkrShaderCompiler_CreateByType(source_code->source_type);
            if (compiler->IsSupportedTargetFormat(format))
            {
                auto& identifier = outResource.option_variants[dyn_hash][fmtIndex];
                auto& stage = identifier.shader_stage;
                // compile & write bytecode to disk
                const auto* shaderImporter = static_cast<SShaderImporter*>(ctx->GetImporter());
                compiler->SetShaderSwitches(flat_static_options, static_variants[static_varidx], static_stable_hashes[static_varidx]);
                compiler->SetShaderOptions(flat_dynamic_options, dynamic_variants[dynamic_varidx], dynamic_stable_hashes[dynamic_varidx]);
                auto compiled = compiler->Compile(format, *source_code, *shaderImporter);
                stage = compiled->GetShaderStage();
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
                        {
                            auto file = fopen(bytesPath.u8string().c_str(), "wb");
                            SKR_DEFER({ fclose(file); });
                            if (!file) SKR_UNREACHABLE_CODE();
                            fwrite(bytes.data(), bytes.size(), 1, file);
                        }
                    }
                    // write pdb to file
                    if (auto pdb = compiled->GetPDB();!pdb.empty())
                    {
                        auto pdbPath = basePath / (fname + ".pdb").c_str();
                        {
                            auto pdb_file = fopen(pdbPath.u8string().c_str(), "wb");
                            SKR_DEFER({ fclose(pdb_file); });
                            if (!pdb_file) SKR_UNREACHABLE_CODE();
                            fwrite(pdb.data(), pdb.size(), 1, pdb_file);
                        }
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
        }); // end foreach target profile
        }); // end foreach dynamic variant
        }); // end foreach variant

    // make resource to write
    auto resource = make_zeroed<skr_platform_shader_collection_resource_t>();
    resource.root_guid = assetRecord->guid;
    // add root variant, root variant has two entries: md5-stable-hash & 0
    {
        const auto root_hash = make_zeroed<skr_stable_shader_hash_t>();
        for (auto&& staticVariant : allOutResources)
        {
            auto& rootOptionVar = staticVariant.GetDynamicVariants(dynamic_stable_hashes[0]);
            staticVariant.option_variants.insert({root_hash, rootOptionVar});
        }
        resource.switch_variants.emplace(root_hash, allOutResources[0]);
    }
    // add shader variants
    for (size_t variant_index = 0u; variant_index < static_variants.size(); variant_index++)
    {
        const auto& variantResource = allOutResources[variant_index];
        resource.switch_variants.emplace(variantResource.stable_hash, variantResource);
    }
    // deserialize
    {
        // make archive
        eastl::vector<uint8_t> resource_data;
        skr::binary::VectorWriter writer{&resource_data};
        skr_binary_writer_t archive(writer);
        skr::binary::Write(&archive, resource);
        // write to file
        auto file = fopen(outputPath.u8string().c_str(), "wb");
        if (!file)
        {
            SKR_LOG_FMT_ERROR("[SShaderCooker::Cook] failed to write cooked file for resource {}! path: {}", 
                assetRecord->guid, assetRecord->path.u8string());
            return false;
        }
        SKR_DEFER({ fclose(file); });
        fwrite(resource_data.data(), resource_data.size(), 1, file);
    }
    // deserialize
    {
        // make archive
        skr_json_writer_t writer(2);
        skr::json::Write(&writer, resource);
        auto jPath = outputPath.u8string() + ".json";
        // write to file
        auto file = fopen(jPath.c_str(), "wb");
        if (!file)
        {
            SKR_LOG_FMT_ERROR("[SShaderCooker::Cook] failed to write cooked file for resource {}! path: {}", 
                assetRecord->guid, assetRecord->path.u8string());
            return false;
        }
        SKR_DEFER({ fclose(file); });
        fwrite(writer.buffer.data(), writer.buffer.size(), 1, file);
    }
    return true;
}

uint32_t SShaderCooker::Version()
{
    return kDevelopmentVersion;
}

}
}