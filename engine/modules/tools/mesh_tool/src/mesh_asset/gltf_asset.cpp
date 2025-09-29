#include "SkrProfile/profile.h"
#include "SkrContainersDef/map.hpp"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrShaderCompiler/assets/material_asset.hpp"
#include "SkrTextureCompiler/texture_compiler.hpp"
#include "SkrMeshTool/mesh_asset.hpp"
#include "SkrMeshTool/gltf_processing.hpp"
#include "cgltf/cgltf.h"

namespace skr
{

void* GltfMeshImporter::Import(skr::io::IRAMService* ioService, CookContext* context)
{
    const auto assetMetaFile = context->GetAssetMetaFile();
    auto path = context->AddSourceFile(assetPath);
    auto vfs = assetMetaFile->GetProject()->GetAssetVFS();
    auto result = SkrNew<ImportData>();
    result->gltf_data = ImportGLTFData(path.string().c_str(), ioService, vfs);
    if (import_all_materials)
    {
        ImportMaterials(result, context);
    }
    return result;
}

void GltfMeshImporter::Destroy(void* resource)
{
    auto import_data = (ImportData*)resource;
    if (import_data->gltf_data)
        ::cgltf_free(import_data->gltf_data);
    SkrDelete(import_data);
}

void GltfMeshImporter::ImportMaterials(ImportData* import_data, CookContext* context)
{
    auto gltf_data = import_data->gltf_data;
    auto project = context->GetAssetMetaFile()->GetProject();
    const auto AssetDirectory = skr::Path(context->GetAssetPath()).parent_directory();
    const auto GLTFDirectory = skr::Path(assetPath).parent_directory();
    auto& CookSystem = *skr::GetCookSystem();

    skr::Map<uint64_t, skr::GUID> ImportedTextures;
    auto ImportTextureOnce = [&](cgltf_texture* t) {
        auto index = cgltf_texture_index(gltf_data, t);
        if (ImportedTextures.contains(index))
            return ImportedTextures.find(index).value();

        skr::GUID TextureAssetID = skr::GUID::Create();
        ImportedTextures.add(index, TextureAssetID);
        auto TextureAssetPath = (AssetDirectory / (const char8_t*)t->image->uri).string();
        auto TextureSourcePath = (GLTFDirectory / (const char8_t*)t->image->uri).string();

        auto TexImporter = Importer::Create<TextureImporter>();
        TexImporter->assetPath = TextureSourcePath;

        auto AssetFile = skr::RC<skr::AssetMetaFile>::New(
            TextureAssetPath,                            // virtual uri for this asset in the project
            TextureAssetID,                              // guid for this asset
            skr::type_id_of<skr::TextureResource>(),     // output resource is a mesh resource
            skr::type_id_of<skr::TextureCooker>() // this cooker cooks t he raw mesh data to mesh resource
        );

        CookSystem.ImportAssetMeta(project, AssetFile, TexImporter);
        import_data->import_textures.add_unique(TextureAssetID);
        return TextureAssetID;
    };

    skr::Map<const cgltf_material*, skr::GUID> ImportedMaterials;
    for (uint32_t i = 0; i < gltf_data->materials_count; i++)
    {
        const auto MaterialAssetID = skr::GUID::Create();
        const auto& material = gltf_data->materials[i];
        ImportedMaterials.add(&material, MaterialAssetID);

        auto MatImporter = Importer::Create<MaterialImporter>();
        MatImporter->asset = skr::SP<MaterialAsset>::New();
        MatImporter->asset->material_type = skr::GUID();
        MatImporter->asset->material_type_version = 0;
        auto pbr = material.pbr_metallic_roughness;

        if (auto basecolor = pbr.base_color_factor)
        {
            auto& v = MatImporter->asset->override_values.emplace().ref();
            v.prop_type = EMaterialPropertyType::FLOAT3;
            v.slot_name = u8"BaseColor";
            v.vec = { basecolor[0], basecolor[1], basecolor[2], basecolor[3] };
        }
        {
            auto& v = MatImporter->asset->override_values.emplace().ref();
            v.prop_type = EMaterialPropertyType::FLOAT;
            v.slot_name = u8"Metallic";
            v.value = pbr.metallic_factor;
        }
        {
            auto& v = MatImporter->asset->override_values.emplace().ref();
            v.prop_type = EMaterialPropertyType::FLOAT;
            v.slot_name = u8"Roughness";
            v.value = pbr.roughness_factor;
        }
        if (auto basecolor = pbr.base_color_texture.texture)
        {
            auto& v = MatImporter->asset->override_values.emplace().ref();
            v.prop_type = EMaterialPropertyType::TEXTURE;
            v.slot_name = u8"BaseColorTexture";
            v.resource = ImportTextureOnce(basecolor);
        }
        if (auto normal = material.normal_texture.texture)
        {
            auto& v = MatImporter->asset->override_values.emplace().ref();
            v.prop_type = EMaterialPropertyType::TEXTURE;
            v.slot_name = u8"NormalMap";
            v.resource = ImportTextureOnce(normal);
        }
        if (auto metal_rough = pbr.metallic_roughness_texture.texture)
        {
            auto& v = MatImporter->asset->override_values.emplace().ref();
            v.prop_type = EMaterialPropertyType::TEXTURE;
            v.slot_name = u8"MetallicRoughness";
            v.resource = ImportTextureOnce(metal_rough);
        }
        if (auto emissive = material.emissive_texture.texture)
        {
            auto& v = MatImporter->asset->override_values.emplace().ref();
            v.prop_type = EMaterialPropertyType::TEXTURE;
            v.slot_name = u8"Emissive";
            v.resource = ImportTextureOnce(emissive);
        }

        auto MaterialAssetPath = (AssetDirectory / (const char8_t*)material.name).string();
        auto AssetFile = skr::RC<skr::AssetMetaFile>::New(
            MaterialAssetPath,                            // virtual uri for this asset in the project
            MaterialAssetID,                              // guid for this asset
            skr::type_id_of<skr::MaterialResource>(),     // output resource is a mesh resource
            skr::type_id_of<skr::MaterialCooker>() // this cooker cooks t he raw mesh data to mesh resource
        );
        CookSystem.ImportAssetMeta(project, AssetFile, MatImporter);
        import_data->import_materials.add_unique(MaterialAssetID);
    }

    {
        SkrZoneScopedN("CookGLTFTexture/Materials");
        skr::Vector<skr::task::event_t> IndirectAssets;
        for (auto [t, TextureAssetID] : ImportedTextures)
            IndirectAssets.add(CookSystem.EnsureCooked(TextureAssetID));

        for (auto [t, MaterialAssetID] : ImportedMaterials)
            IndirectAssets.add(CookSystem.EnsureCooked(MaterialAssetID));

        for (auto e : IndirectAssets)
            e.wait(true);
    }
}

} // namespace skr
