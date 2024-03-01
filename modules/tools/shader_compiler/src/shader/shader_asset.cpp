#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRT/async/parallel_for.hpp"
#include "SkrRT/misc/cartesian_product.hpp"
#include "SkrRT/serde/json/writer.h"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"
#include "SkrShaderCompiler/shader_compiler.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "SkrProfile/profile.h"

namespace skd
{
namespace asset
{
ShaderSourceCode::~ShaderSourceCode() SKR_NOEXCEPT
{

}

void* SShaderImporter::Import(skr_io_ram_service_t* ioService, SCookContext* context)
{
    skr::BlobId ioBlob = nullptr;
    const auto path = context->AddFileDependencyAndLoad(ioService, sourcePath.c_str(), ioBlob);

    // create source code wrapper
    const auto extention = path.extension().u8string();
    const auto source_name = path.filename().replace_extension();
    const auto sourceType = Util_GetShaderSourceTypeWithExtensionString(extention.c_str());
    return SkrNew<ShaderSourceCode>(ioBlob, source_name.u8string().c_str(), sourceType);
}

void SShaderImporter::Destroy(void* resource)
{
    auto source = (ShaderSourceCode*)resource;
    SkrDelete(source);
}

// [x: "on", y: "a", z: "1"]
using unique_option_variant_t = skr::Vector<skr_shader_option_instance_t>;
// [ [z: "on", y: "a", z: "1"], [x: "on", y: "a", z: "2"] ...]
using option_variant_seq_t = skr::Vector<unique_option_variant_t>;
using variant_seq_hashe_seq_t = skr::Vector<skr_stable_shader_hash_t>;
void cartesian_variants(skr::span<skr_shader_options_resource_t*> options, skr::Vector<skr_shader_option_template_t>& out_flatten_options,
option_variant_seq_t& out_variants, variant_seq_hashe_seq_t& out_stable_hahses)
{
    // flat and well sorted
    // [ x: ["on", "off"], y: ["a", "b", "c"], z: ["1", "2"] ]
    skr_shader_options_resource_t::flatten_options(out_flatten_options, options);

    // [ ["on", "off"], ["a", "b", "c"], ["1", "2"] ]
    skr::Vector<skr::Vector<skr::String>> selection_seqs = {};
    selection_seqs.resize_default(out_flatten_options.size());
    for (size_t i = 0u; i < out_flatten_options.size(); ++i)
    {
        selection_seqs[i] = out_flatten_options[i].value_selections;
    }

    // [ [z: "on", y: "a", z: "1"], [x: "on", y: "a", z: "2"] ...]
    (void)out_variants;
    
    if (!selection_seqs.empty())
    {
        // [ ["on", "a", "1"], ["on", "a", "2"] ...]
        skr::cartesian_product<skr::String> cartesian(selection_seqs);
        while (cartesian.has_next())
        {
            skr::Vector<skr_shader_option_instance_t> option_seq = {};
            const auto sequence = cartesian.next();
            option_seq.resize_default(sequence.size());
            SKR_ASSERT(sequence.size() == out_flatten_options.size());
            for (size_t idx = 0u; idx < option_seq.size(); ++idx)
            {
                option_seq[idx].key = out_flatten_options[idx].key;
                option_seq[idx].value = sequence[idx];
            }
            out_variants.add(option_seq);
            const auto stable_hash =
            skr_shader_option_instance_t::calculate_stable_hash({ option_seq.data(), option_seq.size() });
            out_stable_hahses.add(stable_hash);
        }
    }
    else
    {
        out_stable_hahses.add({0u, 0u, 0u, 0u}); // emplace an zero hash
        out_variants.add_default();                    // emplace an empty option sequence
    }
}

// skr_shader_options_resource_t:
// LEVEL["level0", "level1", "level2"]:
//    same as "key": ["level0", "level1", "level2"] but def(level2) includes def(level1) & def(level0))
// SELECT["selection0", "selection1", "selection2"]:
//    same as "key": ["selection0", "selection1", "selection2"]
// SWITCH["switch"]:
//    same as "switch": ["on", "off"]
bool SShaderCooker::Cook(SCookContext* ctx)
{
    SkrZoneScopedN("SShaderCooker::Cook");

    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto source_code = ctx->Import<ShaderSourceCode>();
    SKR_DEFER({ ctx->Destroy(source_code); });
    // Calculate all macro combines (shader variants)
    skr::Vector<skr_shader_options_resource_t*> switch_assets = {};
    skr::Vector<skr_shader_options_resource_t*> option_assets = {};
    auto importer = static_cast<SShaderImporter*>(ctx->GetImporter());
    for (auto switch_asset : importer->switch_assets)
    {
        const auto guid = switch_asset.get_guid();
        auto idx = ctx->AddStaticDependency(guid, true);
        auto opts_resource = static_cast<skr_shader_options_resource_t*>(ctx->GetStaticDependency(idx).get_ptr());
        switch_assets.add(opts_resource);
    }
    for (auto option_asset : importer->option_assets)
    {
        const auto guid = option_asset.get_guid();
        auto idx = ctx->AddStaticDependency(guid, true);
        auto opts_resource = static_cast<skr_shader_options_resource_t*>(ctx->GetStaticDependency(idx).get_ptr());
        option_assets.add(opts_resource);
    }

    skr::Vector<skr_shader_option_template_t> flat_static_options = {};
    option_variant_seq_t static_variants = {};
    variant_seq_hashe_seq_t static_stable_hashes = {};
    cartesian_variants(switch_assets, flat_static_options, static_variants, static_stable_hashes);

    skr::Vector<skr_shader_option_template_t> flat_dynamic_options = {};
    option_variant_seq_t dynamic_variants = {};
    variant_seq_hashe_seq_t dynamic_stable_hashes = {};
    cartesian_variants(option_assets, flat_dynamic_options, dynamic_variants, dynamic_stable_hashes);

    // Enumerate destination bytecode format
    // TODO: REFACTOR THIS
    skr::Vector<ECGPUShaderBytecodeType> byteCodeFormats = {
        ECGPUShaderBytecodeType::CGPU_SHADER_BYTECODE_TYPE_DXIL,
        ECGPUShaderBytecodeType::CGPU_SHADER_BYTECODE_TYPE_SPIRV
    };
    // begin compile
    // auto system = skd::asset::GetCookSystem();
    skr::Vector<skr_multi_shader_resource_t> allOutResources(static_variants.size());
    // foreach variants
    {
    SkrZoneScopedN("Permutations::Compile");
    skr::parallel_for(static_variants.begin(), static_variants.end(), 1,
    [&](const auto* pVariant, const auto* _) -> void {
        const auto* shaderImporter = static_cast<SShaderImporter*>(ctx->GetImporter());
        const uint64_t static_varidx = pVariant - static_variants.begin();
        auto& outResource = allOutResources[static_varidx];
        outResource.entry = shaderImporter->entry;
        outResource.stable_hash = static_stable_hashes[static_varidx];
        for (const auto dyn_hash : dynamic_stable_hashes)
        {
            outResource.option_variants[dyn_hash] = {};
            outResource.option_variants[dyn_hash].resize_default(byteCodeFormats.size());
        }
        SkrZoneScopedN("StaticPermutations::Compile");

        // foreach dynamic variants
        skr::parallel_for(dynamic_variants.begin(), dynamic_variants.end(), 1,
        [&](const auto* pDynamicVariant, const auto* __) -> void {
            const uint64_t dynamic_varidx = pDynamicVariant - dynamic_variants.begin();
            const auto dyn_hash = dynamic_stable_hashes[dynamic_varidx];
            SkrZoneScopedN("DynamicPermutations::Compile");

            // foreach target profiles
            skr::parallel_for(byteCodeFormats.begin(), byteCodeFormats.end(), 1,
            [&](const ECGPUShaderBytecodeType* pFormat, const ECGPUShaderBytecodeType* ___) -> void {
                SkrZoneScopedN("ShaderCompileTask");

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
                        const auto fname = skr::format(u8"{}#{}-{}-{}-{}",
                        identifier.hash.flags, identifier.hash.encoded_digits[0],
                        identifier.hash.encoded_digits[1], identifier.hash.encoded_digits[2], identifier.hash.encoded_digits[3]);
                        // create dir
                        std::error_code ec = {};
                        skr::filesystem::create_directories(basePath, ec);
                        // write bytes to file
                        {
                            auto bytesPath = basePath / skr::format(u8"{}.bytes", fname).c_str();
                            {
                                auto file = fopen(bytesPath.string().c_str(), "wb");
                                SKR_DEFER({ fclose(file); });
                                if (!file)
                                {
                                    int err_num = errno;
                                    SKR_LOG_FATAL(u8"Open Shader Output File errno = %d, reason = %s!", err_num, ::strerror(err_num));
                                    SKR_UNREACHABLE_CODE();
                                }
                                fwrite(bytes.data(), bytes.size(), 1, file);
                            }
                        }
                        // write pdb to file
                        if (auto pdb = compiled->GetPDB(); !pdb.empty())
                        {
                            auto pdbPath = basePath / skr::format(u8"{}.pdb", fname).c_str();
                            {
                                auto pdb_file = fopen(pdbPath.string().c_str(), "wb");
                                SKR_DEFER({ fclose(pdb_file); });
                                if (!pdb_file) 
                                {
                                    int err_num = errno;
                                    SKR_LOG_FATAL(u8"Open PDB File errno = %d, reason = %s!", err_num, ::strerror(err_num));
                                    SKR_UNREACHABLE_CODE();
                                }
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
                }
                SkrShaderCompiler_Destroy(compiler);
            }); // end foreach target profile
        });     // end foreach dynamic variant
    });         // end foreach variant
    }
    
    // resolve output stage
    for (auto&& staticVariant : allOutResources)
    {
        for (auto&& multiShader : staticVariant.option_variants)
        {
            const ECGPUShaderStage stage = multiShader.second[0].shader_stage; 
            for (auto&& identifier : multiShader.second)
            {
                SKR_ASSERT(stage == identifier.shader_stage);
                break;
            }
            staticVariant.shader_stage = stage;
        }
    }

    // make resource to write
    skr_shader_collection_resource_t resource = {};
    auto switches_blob = skr::make_blob_builder<skr_shader_option_sequence_t>();
    auto options_blob = skr::make_blob_builder<skr_shader_option_sequence_t>();
    // initialize & serialize
    {
        resource.root_guid = assetRecord->guid;
        // add root variant, root variant has two entries: md5-stable-hash & 0
        {
            const auto root_hash = make_zeroed<skr_stable_shader_hash_t>();
            for (auto&& staticVariant : allOutResources)
            {
                auto& rootOptionVar = staticVariant.GetDynamicVariants(dynamic_stable_hashes[0]);
                staticVariant.option_variants.insert({ root_hash, rootOptionVar });
            }
            resource.switch_variants.emplace(root_hash, allOutResources[0]);
        }
        // add shader variants
        for (size_t variant_index = 0u; variant_index < static_variants.size(); variant_index++)
        {
            const auto& variantResource = allOutResources[variant_index];
            resource.switch_variants.emplace(variantResource.stable_hash, variantResource);
        }
        // add static seq
        for (auto&& static_switch : flat_static_options)
        {
            switches_blob.types.add(static_switch.type);
            switches_blob.keys.add(static_switch.key);
            auto& values = switches_blob.values.add_default().ref();
            for (const auto& value : static_switch.value_selections)
            {
                values.add(value);
            }
        }
        for (auto&& option_switch : flat_dynamic_options)
        {
            options_blob.types.add(option_switch.type);
            options_blob.keys.add(option_switch.key);
            auto& values = options_blob.values.add_default().ref();
            for (const auto& value : option_switch.value_selections)
            {
                values.add(value);
            }
        }
        resource.switch_arena = skr::binary::make_arena<skr_shader_option_sequence_t>(resource.switch_sequence, switches_blob);
        resource.option_arena = skr::binary::make_arena<skr_shader_option_sequence_t>(resource.option_sequence, options_blob);

        ctx->Save(resource);
    }
    // serialize a json file for visual debugging
    {
        skr_shader_collection_json_t json_resource = {};
        json_resource.root_guid = resource.root_guid;
        json_resource.switch_variants = resource.switch_variants;

        json_resource.switch_type_sequence = switches_blob.types;
        json_resource.switch_key_sequence = switches_blob.keys;
        json_resource.switch_values_sequence = switches_blob.values;
        
        json_resource.option_type_sequence = options_blob.types;
        json_resource.option_key_sequence = options_blob.keys;
        json_resource.option_values_sequence = options_blob.values;

        // make archive
        skr_json_writer_t writer(2);
        skr::json::Write(&writer, json_resource);
        auto jPath = outputPath.string() + ".json";
        // write to file
        auto file = fopen(jPath.c_str(), "wb");
        if (!file)
        {
            SKR_LOG_FMT_ERROR(u8"[SShaderCooker::Cook] failed to write cooked file for json_resource {}! path: {}",
            assetRecord->guid, assetRecord->path.string());
            return false;
        }
        SKR_DEFER({ fclose(file); });
        fwrite(writer.buffer.c_str(), writer.buffer.size(), 1, file);
    }
    return true;
}

uint32_t SShaderCooker::Version()
{
    return kDevelopmentVersion;
}

} // namespace asset
} // namespace skd