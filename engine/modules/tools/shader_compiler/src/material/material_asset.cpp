#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/material_asset.hpp"

namespace skr
{

MaterialImporter::MaterialImporter()
{
}

MaterialImporter::~MaterialImporter()
{
    asset = nullptr;
}

MaterialAsset::~MaterialAsset()
{
    material_type_version = 0;
}

void* MaterialImporter::Import(skr::io::IRAMService* ioService, CookContext* context)
{
    if (!asset)
    {
        skr::BlobId blob = nullptr;
        context->AddSourceFileAndLoad(ioService, jsonPath.c_str(), blob);
        SKR_DEFER({ blob.reset(); });
        /*
        const auto assetMetaFile = context->GetAssetMetaFile();
        {
            SKR_LOG_FMT_ERROR(u8"Import shader options asset {} from {} failed, json parse error {}", assetMetaFile->guid, jsonPath, ::error_message(doc.error()));
            return nullptr;
        }
        '*/

        auto reader = skr::ArReadJson::ReadBuffer(blob->get_data(), blob->get_size());
        auto mat_asset = SkrNew<MaterialAsset>();
        reader.value(*mat_asset);
        return mat_asset;
    }
    return asset.get();
}

void MaterialImporter::Destroy(void* resource)
{
    if (asset)
    {
    }
    else
    {
        auto mat_asset = (MaterialAsset*)resource;
        SkrDelete(mat_asset);
    }
}

bool MaterialCooker::Cook(CookContext* ctx)
{
    //-----load config
    // no cook config for config, skipping

    //-----import resource object
    auto material = ctx->Import<MaterialAsset>();
    if (!material) return false;
    SKR_DEFER({ ctx->Destroy(material); });

    // convert to runtime resource
    MaterialResource runtime_material;
    runtime_material.material_type_version = material->material_type_version;

    if (!material->material_type.is_null())
    {
        runtime_material.material_type = material->material_type.get_guid();
        auto idx = ctx->AddStaticDependency(runtime_material.material_type.get_guid(), true);
        ctx->AddRuntimeDependency(runtime_material.material_type.get_guid());
        const auto& rhandle = ctx->GetStaticDependency(idx);

        auto matType = static_cast<MaterialTypeResource*>(rhandle.get_installed());
        // calculate switch macros for material & place variants
        for (auto& pass : matType->passes)
        {
            for (auto& shader_resource : pass.shader_resources)
            {
                auto& variant = runtime_material.overrides.switch_variants.add_default().ref();

                // initiate static switches to a permutation in shader collection
                const auto shader_collection = shader_resource.load();
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
                auto switch_indices_span = skr::Span<uint32_t>(variant.switch_indices.data(), variant.switch_indices.size());
                auto option_indices_span = skr::Span<uint32_t>(variant.option_indices.data(), variant.option_indices.size());
                const auto switch_hash = ShaderOptionSequence::calculate_stable_hash(shader_collection->switch_sequence, switch_indices_span);
                const auto option_hash = ShaderOptionSequence::calculate_stable_hash(shader_collection->option_sequence, option_indices_span);

                variant.shader_collection = shader_resource.get_guid();
                variant.switch_hash = switch_hash;
                variant.option_hash = option_hash;
            }
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
    }

    // TODO: check & validate material overrides

    // value overrides
    for (const auto& prop : material->override_values)
    {
        using namespace skr;

        switch (prop.prop_type)
        {
        case EMaterialPropertyType::BOOL: 
            runtime_material.SetBoolParameterValue(prop.slot_name, (bool)prop.value);
            break;
        case EMaterialPropertyType::FLOAT: 
            runtime_material.SetFloatParameterValue(prop.slot_name, (float)prop.value);
            break;
        case EMaterialPropertyType::FLOAT2: 
            runtime_material.SetFloat2ParameterValue(prop.slot_name, prop.vec.xy());
            break;
        case EMaterialPropertyType::FLOAT3: 
            runtime_material.SetFloat3ParameterValue(prop.slot_name, prop.vec.xyz());
            break;
        case EMaterialPropertyType::FLOAT4:
            runtime_material.SetFloat4ParameterValue(prop.slot_name, prop.vec.xyzw());
            break;
        case EMaterialPropertyType::DOUBLE:
            runtime_material.SetDoubleParameterValue(prop.slot_name, (double)prop.value);
            break;
        case EMaterialPropertyType::TEXTURE: {
            runtime_material.SetTextureParameterValue(prop.slot_name, prop.resource);
            ctx->AddRuntimeDependency(prop.resource.get_guid());
        }
        break;
        case EMaterialPropertyType::SAMPLER: {
            runtime_material.SetSamplerParameterValue(prop.slot_name, prop.resource);
            ctx->AddRuntimeDependency(prop.resource.get_guid());
        }
        break;
        case EMaterialPropertyType::BUFFER: {
            SKR_UNIMPLEMENTED_FUNCTION();
        }
        break;
        default: {
            SKR_ASSERT(false && "Unsupported material property type");
        }
        break;
        }
    }
    return ctx->Save(runtime_material);
}
} // namespace skr