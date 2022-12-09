#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrShaderCompiler/assets/material_asset.hpp"
#include "utils/io.h"
#include "platform/debug.h"
#include "SkrToolCore/project/project.hpp"
#include "json/reader.h"

namespace skd
{
namespace asset
{

void* SMaterialImporter::Import(skr_io_ram_service_t* ioService, SCookContext *context)
{
    const auto assetRecord = context->GetAssetRecord();
    skr_async_ram_destination_t destination = {};
    context->AddFileDependencyAndLoad(ioService, jsonPath.c_str(), destination);
    SKR_DEFER({sakura_free(destination.bytes);});

    auto jsonString = simdjson::padded_string((char8_t*)destination.bytes, destination.size);
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    if(doc.error())
    {
        SKR_LOG_FMT_ERROR("Import shader options asset {} from {} failed, json parse error {}", assetRecord->guid, jsonPath, simdjson::error_message(doc.error()));
        return nullptr;
    }
    auto json_value = doc.get_value().value_unsafe();

    // create source code wrapper
    auto mat_asset = SkrNew<skr_material_asset_t>();
    skr::json::Read(std::move(json_value), *mat_asset);
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
    auto matType= static_cast<skr_material_type_resource_t*>(ctx->GetStaticDependency(idx).get_ptr());
    auto blob = skr::make_blob_builder<skr_material_overrides_t>();
    
    // calculate switch macros for material & place variants
    for (auto& shader_resource : matType->shader_resources)
    {
        eastl::vector<skr_shader_option_t> flat_switches = {};
        shader_resource.resolve(false, nullptr);
        const auto shader_collection = shader_resource.get_ptr();
        (void)shader_collection;
        // skr_shader_options_resource_t::flatten_options(flat_switches, );
    }


    // value overrides
    for (const auto& prop : material->override_values)
    {
        switch (prop.prop_type)
        {
            case ESkrMaterialPropertyType::BOOL:
            {
                auto vblob = skr::make_blob_builder<skr_material_value_bool_t>();
                vblob.slot_name = prop.slot_name;
                vblob.value = (bool)prop.value;
                blob.bools.emplace_back(vblob);
            }
            break;
            case ESkrMaterialPropertyType::FLOAT:
            {
                auto vblob = skr::make_blob_builder<skr_material_value_float_t>();
                vblob.slot_name = prop.slot_name;
                vblob.value = (float)prop.value;
                blob.floats.emplace_back(vblob);
            }
            break;
            case ESkrMaterialPropertyType::FLOAT2:
            {
                auto vblob = skr::make_blob_builder<skr_material_value_float2_t>();
                vblob.slot_name = prop.slot_name;
                vblob.value = { (float)prop.vec.x, (float)prop.vec.y };
                blob.float2s.emplace_back(vblob);
            }
            break;
            case ESkrMaterialPropertyType::FLOAT3:
            {
                auto vblob = skr::make_blob_builder<skr_material_value_float3_t>();
                vblob.slot_name = prop.slot_name;
                vblob.value = { (float)prop.vec.x, (float)prop.vec.y, (float)prop.vec.z };
                blob.float3s.emplace_back(vblob);
            }
            break;
            case ESkrMaterialPropertyType::FLOAT4:
            {
                auto vblob = skr::make_blob_builder<skr_material_value_float4_t>();
                vblob.slot_name = prop.slot_name;
                vblob.value = { (float)prop.vec.x, (float)prop.vec.y, (float)prop.vec.z, (float)prop.vec.w };
                blob.float4s.emplace_back(vblob);
            }
            break;
            case ESkrMaterialPropertyType::DOUBLE:
            {
                auto vblob = skr::make_blob_builder<skr_material_value_double_t>();
                vblob.slot_name = prop.slot_name;
                vblob.value = prop.value;
                blob.doubles.emplace_back(vblob);
            }
            break;
            case ESkrMaterialPropertyType::TEXTURE:
            {
                auto vblob = skr::make_blob_builder<skr_material_value_texture_t>();
                vblob.slot_name = prop.slot_name;
                vblob.value = prop.resource.get_guid();
                blob.textures.emplace_back(vblob);
            }
            break;
            case ESkrMaterialPropertyType::BUFFER:
            case ESkrMaterialPropertyType::SAMPLER:
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

    runtime_material.arena = skr::binary::make_arena<skr_material_overrides_t>(runtime_material.overrides, blob);

    return ctx->Save(runtime_material);
}
}
}