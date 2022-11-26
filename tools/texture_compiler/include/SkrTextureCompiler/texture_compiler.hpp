#pragma once
#include "platform/configure.h"
#include "SkrTextureCompiler/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#ifndef __meta__
    #include "SkrTextureCompiler/texture_compiler.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "a26c2436-9e5f-43c4-b4d7-e5373d353bae")
sattr("serialize" : "json")
SKR_TEXTURE_COMPILER_API STextureImporter final : public SImporter
{
    sattr("no-default" : true)
    skr::string assetPath;

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
sregister_cooker("f8821efb-f027-4367-a244-9cc3efb3a3bf");

struct SKR_TEXTURE_COMPILER_API STextureImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override;
    skr_guid_t GetResourceType() override;
    void CreateImporter(const SAssetRecord* record) override;
};

}
}