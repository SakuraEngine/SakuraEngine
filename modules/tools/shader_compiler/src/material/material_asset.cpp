#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/material_asset.hpp"

namespace skd
{
namespace asset
{

void* SMaterialImporter::Import(skr_io_ram_service_t* ioService, SCookContext *context)
{
    skr::BlobId blob = nullptr;
    context->AddSourceFileAndLoad(ioService, jsonPath.c_str(), blob);
    SKR_DEFER({blob.reset();});
    /*
    const auto assetRecord = context->GetAssetRecord();
    {
        SKR_LOG_FMT_ERROR(u8"Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, ::error_message(doc.error()));
        return nullptr;
    }
    '*/
    skr::String jString(skr::StringView((const char8_t*)blob->get_data(), blob->get_size()));
    skr::archive::JsonReader jsonVal(jString.view());
    auto mat_asset = SkrNew<skr_material_asset_t>();
    skr::json::Read(&jsonVal, *mat_asset);
    return mat_asset;
}

void SMaterialImporter::Destroy(void *resource)
{
    auto mat_asset = (skr_material_asset_t*)resource;
    SkrDelete(mat_asset);
}

bool SMaterialCooker::Cook(SCookContext *ctx)
{
    const auto outputPath = ctx->GetOutputPath();
    //-----load config
    // no cook config for config, skipping

    //-----import resource object
    auto material = ctx->Import<skr_material_asset_t>();
    if(!material) return false;
    SKR_DEFER({ ctx->Destroy(material); });

    // convert to runtime resource
    skr_material_resource_t runtime_material;
    runtime_material.material_type_version = material->material_type_version;
    runtime_material.material_type = material->material_type.get_guid();

    auto idx = ctx->AddStaticDependency(runtime_material.material_type.get_guid(), true);
    ctx->AddRuntimeDependency(runtime_material.material_type.get_guid());
    const auto& rhandle = ctx->GetStaticDependency(idx);
    auto matType= static_cast<skr_material_type_resource_t*>(rhandle.get_ptr());
    // calculate switch macros for material & place variants
    for (auto& pass : matType->passes)
    for (auto& shader_resource : pass.shader_resources)
    {
        auto& variant = runtime_material.overrides.switch_variants.add_default().ref(); 

        shader_resource.resolve(false, nullptr);
        // initiate static switches to a permutation in shader collection 
        const auto shader_collection = shader_resource.get_ptr();
        variant.switch_indices.resize_default(shader_collection->switch_sequence.keys.size());
        variant.option_indices.resize_default(shader_collection->option_sequence.keys.size());
        // calculate final values for static switches
        for (uint32_t switch_i = 0; switch_i < variant.switch_indices.size(); switch_i++)
        {
            // default value
            const auto& default_value = matType->switch_defaults[switch_i];
            const auto default_index = shader_collection->switch_sequence.find_value_index(default_value.key.u8_str(), default_value.value.u8_str());
            SKR_ASSERT(default_index != UINT32_MAX && "Invalid switch default value");
            variant.switch_indices[switch_i] = default_index;
            // TODO: override
        }
        // calculate final asset values for options
        for (uint32_t option_i = 0; option_i < variant.option_indices.size(); option_i++)
        {
            // default value
            const auto& default_value = matType->option_defaults[option_i];
            const auto default_index = shader_collection->option_sequence.find_value_index(default_value.key.u8_str(), default_value.value.u8_str());
            SKR_ASSERT(default_index != UINT32_MAX && "Invalid option default value");
            variant.option_indices[option_i] = default_index;
            // TODO: override
        }
        // calculate hashes and record
        auto switch_indices_span = skr::span<uint32_t>(variant.switch_indices.data(), variant.switch_indices.size());
        auto option_indices_span = skr::span<uint32_t>(variant.option_indices.data(), variant.option_indices.size());
        const auto switch_hash = skr_shader_option_sequence_t::calculate_stable_hash(shader_collection->switch_sequence, switch_indices_span);
        const auto option_hash = skr_shader_option_sequence_t::calculate_stable_hash(shader_collection->option_sequence, option_indices_span);
    
        variant.shader_collection = shader_resource.get_record()->header.guid;
        variant.switch_hash = switch_hash;
        variant.option_hash = option_hash;
    }

    // if material->overrides do not include a value, use default variant in material type
    for (const auto& default_value : matType->default_values)
    {
        bool overrided = false;
        for (const auto& override_value : material->override_values)
        {
            if (default_value.slot_name == override_value.slot_name)
            {
                overrided = true;
                break;
            }
        }
        if (!overrided)
        {
            material->override_values.add(default_value);
        }
    }

    // TODO: check & validate material overrides


    // value overrides
    for (const auto& prop : material->override_values)
    {
        using namespace skr::renderer;
        
        switch (prop.prop_type)
        {
            case EMaterialPropertyType::BOOL:
            {
                MaterialValueBool value = {
                    .slot_name = prop.slot_name,
                    .value = (bool)prop.value
                };
                runtime_material.overrides.bools.add(value);
            }
            break;
            case EMaterialPropertyType::FLOAT:
            {
                MaterialValueFloat value = {
                    .slot_name = prop.slot_name,
                    .value = (float)prop.value
                };
                runtime_material.overrides.floats.add(value);
            }
            break;
            case EMaterialPropertyType::FLOAT2:
            {
                MaterialValueFloat2 value = {
                    .slot_name = prop.slot_name,
                    .value = { (float)prop.vec.x, (float)prop.vec.y }
                };
                runtime_material.overrides.float2s.add(value);
            }
            break;
            case EMaterialPropertyType::FLOAT3:
            {
                MaterialValueFloat3 value = {
                    .slot_name = prop.slot_name,
                    .value = { (float)prop.vec.x, (float)prop.vec.y, (float)prop.vec.z }
                };
                runtime_material.overrides.float3s.add(value);
            }
            break;
            case EMaterialPropertyType::FLOAT4:
            {
                MaterialValueFloat4 value = {
                    .slot_name = prop.slot_name,
                    .value = { (float)prop.vec.x, (float)prop.vec.y, (float)prop.vec.z, (float)prop.vec.w }
                };
                runtime_material.overrides.float4s.add(value);
            }
            break;
            case EMaterialPropertyType::DOUBLE:
            {
                MaterialValueDouble value = {
                    .slot_name = prop.slot_name,
                    .value = (double)prop.value
                };
                runtime_material.overrides.doubles.add(value);
            }
            break;
            case EMaterialPropertyType::TEXTURE:
            {
                MaterialValueTexture value = {
                    .slot_name = prop.slot_name,
                    .value = prop.resource.get_guid()
                };
                runtime_material.overrides.textures.add(value);

                // Add runtime resource dependency
                ctx->AddRuntimeDependency(prop.resource.get_guid());
            }
            break;
            case EMaterialPropertyType::SAMPLER:
            {
                MaterialValueSampler value = {
                    .slot_name = prop.slot_name,
                    .value = prop.resource.get_guid()
                };
                runtime_material.overrides.samplers.add(value);

                // Add runtime resource dependency
                ctx->AddRuntimeDependency(prop.resource.get_guid());
            }
            break;
            case EMaterialPropertyType::BUFFER:
            {
                SKR_UNIMPLEMENTED_FUNCTION();
            }
            break;
            default:
            {
                SKR_ASSERT(false && "Unsupported material property type");
            }
            break;
        }
    }
    return ctx->Save(runtime_material);
}
}
}