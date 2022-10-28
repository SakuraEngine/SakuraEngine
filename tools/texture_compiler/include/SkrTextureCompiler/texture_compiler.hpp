#pragma once
#include "platform/configure.h"
#include "SkrTextureCompiler/module.configure.h"
#include "asset/importer.hpp"
#ifndef __meta__
    #include "SkrTextureCompiler/texture_compiler.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
struct sreflect sattr(
    "guid" : "a26c2436-9e5f-43c4-b4d7-e5373d353bae",
    "serialize" : "json"
)
SKR_TEXTURE_COMPILER_API STextureImporter final : public SImporter
{
    eastl::string assetPath;

    void* Import(skr::io::RAMService*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sregister_importer();

struct sreflect
SKR_TEXTURE_COMPILER_API STextureCooker final : public SCooker 
{ 
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker("b9f81b6f-b544-46e1-8a80-b3269a1c2386");

struct SKR_TEXTURE_COMPILER_API STextureImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override;
    skr_guid_t GetResourceType() override;
    void CreateImporter(const SAssetRecord* record) override;
};

}
}