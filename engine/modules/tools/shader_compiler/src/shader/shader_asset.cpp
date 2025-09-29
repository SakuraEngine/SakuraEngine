#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrRuntime/misc/cartesian_product.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"
#include "SkrShaderCompiler/shader_compiler.hpp"

#include <stdio.h>
#include <stdlib.h>

#include "SkrProfile/profile.h"

namespace skr
{
ShaderSourceCode::~ShaderSourceCode() SKR_NOEXCEPT
{
}

void* ShaderImporter::Import(skr::io::IRAMService* ioService, CookContext* context)
{
    skr::BlobId ioBlob = nullptr;
    const auto path = context->AddSourceFileAndLoad(ioService, sourcePath.c_str(), ioBlob);

    // create source code wrapper
    const skr::Path pathObj{ path };
    const auto extention = pathObj.extension(false);
    const auto source_name = pathObj.basename();
    const auto sourceType = Util_GetShaderSourceTypeWithExtensionString(extention.string().c_str());
    return SkrNew<ShaderSourceCode>(ioBlob, source_name.string().c_str(), sourceType);
}

void ShaderImporter::Destroy(void* resource)
{
    auto source = (ShaderSourceCode*)resource;
    SkrDelete(source);
}

// [x: "on", y: "a", z: "1"]
using unique_option_variant_t = skr::Vector<ShaderOptionInstance>;
// [ [z: "on", y: "a", z: "1"], [x: "on", y: "a", z: "2"] ...]
using option_variant_seq_t = skr::Vector<unique_option_variant_t>;
using variant_seq_hashe_seq_t = skr::Vector<StableShaderHash>;
void cartesian_variants(skr::Span<ShaderOptionsResource*> options, skr::Vector<ShaderOptionTemplate>& out_flatten_options, option_variant_seq_t& out_variants, variant_seq_hashe_seq_t& out_stable_hahses)
{
    // flat and well sorted
    // [ x: ["on", "off"], y: ["a", "b", "c"], z: ["1", "2"] ]
    ShaderOptionsResource::flatten_options(out_flatten_options, options);

    // [ ["on", "off"], ["a", "b", "c"], ["1", "2"] ]
    skr::Vector<skr::Vector<skr::String>> selection_seqs = {};
    selection_seqs.resize_default(out_flatten_options.size());
    for (size_t i = 0u; i < out_flatten_options.size(); ++i)
    {
        selection_seqs[i] = out_flatten_options[i].value_selections;
    }

    // [ [z: "on", y: "a", z: "1"], [x: "on", y: "a", z: "2"] ...]
    (void)out_variants;

    if (!selection_seqs.is_empty())
    {
        // [ ["on", "a", "1"], ["on", "a", "2"] ...]
        skr::cartesian_product<skr::String> cartesian(selection_seqs);
        while (cartesian.has_next())
        {
            skr::Vector<ShaderOptionInstance> option_seq = {};
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
                ShaderOptionInstance::calculate_stable_hash({ option_seq.data(), option_seq.size() });
            out_stable_hahses.add(stable_hash);
        }
    }
    else
    {
        out_stable_hahses.add({ 0u, 0u, 0u, 0u }); // emplace an zero hash
        out_variants.add_default();                // emplace an empty option sequence
    }
}

// ShaderOptionsResource:
// LEVEL["level0", "level1", "level2"]:
//    same as "key": ["level0", "level1", "level2"] but def(level2) includes def(level1) & def(level0))
// SELECT["selection0", "selection1", "selection2"]:
//    same as "key": ["selection0", "selection1", "selection2"]
// SWITCH["switch"]:
//    same as "switch": ["on", "off"]
bool ShaderCooker::Cook(CookContext* ctx)
{
    SkrZoneScopedN("ShaderCooker::Cook");

    const auto assetMetaFile = ctx->GetAssetMetaFile();
    auto source_code = ctx->Import<ShaderSourceCode>();
    SKR_DEFER({ ctx->Destroy(source_code); });
    // Calculate all macro combines (shader variants)
    skr::Vector<ShaderOptionsResource*> switch_assets = {};
    skr::Vector<ShaderOptionsResource*> option_assets = {};
    auto importer = static_cast<ShaderImporter*>(ctx->GetImporter());
    for (auto switch_asset : importer->switch_assets)
    {
        const auto guid = switch_asset.get_guid();
        auto idx = ctx->AddStaticDependency(guid, true);
        auto opts_resource = static_cast<ShaderOptionsResource*>(ctx->GetStaticDependency(idx).get_installed());
        switch_assets.add(opts_resource);
    }
    for (auto option_asset : importer->option_assets)
    {
        const auto guid = option_asset.get_guid();
        auto idx = ctx->AddStaticDependency(guid, true);
        auto opts_resource = static_cast<ShaderOptionsResource*>(ctx->GetStaticDependency(idx).get_installed());
        option_assets.add(opts_resource);
    }

    skr::Vector<ShaderOptionTemplate> flat_static_options = {};
    option_variant_seq_t static_variants = {};
    variant_seq_hashe_seq_t static_stable_hashes = {};
    cartesian_variants(switch_assets, flat_static_options, static_variants, static_stable_hashes);

    skr::Vector<ShaderOptionTemplate> flat_dynamic_options = {};
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
    // auto system = skr::GetCookSystem();
    skr::Vector<MultiShaderResource> allOutResources(static_variants.size());
    // foreach variants
    {
        SkrZoneScopedN("Permutations::Compile");
        skr::parallel_for(static_variants.begin(), static_variants.end(), 1, [&](const auto* pVariant, const auto* _) -> void {
            const auto* shaderImporter = static_cast<ShaderImporter*>(ctx->GetImporter());
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
            skr::parallel_for(dynamic_variants.begin(), dynamic_variants.end(), 1, [&](const auto* pDynamicVariant, const auto* __) -> void {
                const uint64_t dynamic_varidx = pDynamicVariant - dynamic_variants.begin();
                const auto dyn_hash = dynamic_stable_hashes[dynamic_varidx];
                SkrZoneScopedN("DynamicPermutations::Compile");

                // foreach target profiles
                skr::parallel_for(byteCodeFormats.begin(), byteCodeFormats.end(), 1, [&](const ECGPUShaderBytecodeType* pFormat, const ECGPUShaderBytecodeType* ___) -> void {
                    SkrZoneScopedN("ShaderCompileTask");

                    const ECGPUShaderBytecodeType format = *pFormat;
                    const uint64_t fmtIndex = pFormat - byteCodeFormats.begin();

                    auto compiler = SkrShaderCompiler_CreateByType(source_code->source_type);
                    if (compiler->IsSupportedTargetFormat(format))
                    {
                        auto& identifier = outResource.option_variants[dyn_hash][fmtIndex];
                        auto& stage = identifier.shader_stage;
                        // compile & write bytecode to disk
                        const auto* shaderImporter = static_cast<ShaderImporter*>(ctx->GetImporter());
                        compiler->SetShaderSwitches(flat_static_options, static_variants[static_varidx], static_stable_hashes[static_varidx]);
                        compiler->SetShaderOptions(flat_dynamic_options, dynamic_variants[dynamic_varidx], dynamic_stable_hashes[dynamic_varidx]);
                        auto compiled = compiler->Compile(format, *source_code, *shaderImporter);
                        stage = compiled->GetShaderStage();
                        auto bytes = compiled->GetBytecode();
                        auto hashed = compiled->GetHashCode(&identifier.hash.flags, identifier.hash.encoded_digits);
                        if (hashed && !bytes.is_empty())
                        {
                            // write bytecode to disk using ResourceVFS
                            const auto subdir = CGPUShaderBytecodeTypeNames[format];
                            const auto fname = skr::format(u8"{}#{}-{}-{}-{}", identifier.hash.flags, identifier.hash.encoded_digits[0], identifier.hash.encoded_digits[1], identifier.hash.encoded_digits[2], identifier.hash.encoded_digits[3]);

                            // write bytes to file
                            {
                                auto bytesFilename = skr::format(u8"{}/{}.bytes", subdir, fname);
                                if (!ctx->SaveExtra(bytes, bytesFilename.c_str()))
                                {
                                    SKR_LOG_FATAL(u8"Failed to save shader bytecode!");
                                    SKR_UNREACHABLE_CODE();
                                }
                            }

                            // write pdb to file
                            if (auto pdb = compiled->GetPDB(); !pdb.is_empty())
                            {
                                auto pdbFilename = skr::format(u8"{}/{}.pdb", subdir, fname);
                                if (!ctx->SaveExtra(pdb, pdbFilename.c_str()))
                                {
                                    SKR_LOG_ERROR(u8"Failed to save shader PDB!");
                                    // PDB save failure is not fatal
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
    ShaderCollectionResource resource = {};
    ShaderOptionSequence& switches = resource.switch_sequence;
    ShaderOptionSequence& options = resource.option_sequence;
    // initialize & serialize
    {
        resource.root_guid = assetMetaFile->GetGUID();
        // add root variant, root variant has two entries: md5-stable-hash & 0
        {
            const auto root_hash = make_zeroed<StableShaderHash>();
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
            switches.types.add(static_switch.type);
            switches.keys.add(static_switch.key);
            auto& values = switches.values.add_default().ref();
            for (const auto& value : static_switch.value_selections)
            {
                values.add(value);
            }
        }
        for (auto&& option_switch : flat_dynamic_options)
        {
            options.types.add(option_switch.type);
            options.keys.add(option_switch.key);
            auto& values = options.values.add_default().ref();
            for (const auto& value : option_switch.value_selections)
            {
                values.add(value);
            }
        }
        ctx->Save(resource);
    }
    // serialize a json file for visual debugging
    {
        ShaderCollectionJSON json_resource = {};
        json_resource.root_guid = resource.root_guid;
        json_resource.switch_variants = resource.switch_variants;

        json_resource.switch_type_sequence = switches.types;
        json_resource.switch_key_sequence = switches.keys;
        json_resource.switch_values_sequence = switches.values;

        json_resource.option_type_sequence = options.types;
        json_resource.option_key_sequence = options.keys;
        json_resource.option_values_sequence = options.values;

        // make archive
        auto writer = skr::ArWriteJson::Create();
        writer.value(json_resource);
        skr::String jString;
        writer.write_to_string(jString);

        // write to file using ResourceVFS
        auto jsonFilename = skr::format(u8"{}.json", assetMetaFile->GetGUID());
        skr::Span<const uint8_t> jsonData{ reinterpret_cast<const uint8_t*>(jString.c_str_raw()), jString.length_buffer() };
        if (!ctx->SaveExtra(jsonData, jsonFilename.c_str()))
        {
            SKR_LOG_FMT_ERROR(u8"[ShaderCooker::Cook] failed to write json file for resource {}!", assetMetaFile->GetGUID());
            return false;
        }
    }
    return true;
}

uint32_t ShaderCooker::Version()
{
    return kDevelopmentVersion;
}

} // namespace skr