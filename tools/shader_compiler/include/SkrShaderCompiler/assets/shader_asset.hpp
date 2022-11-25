#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "platform/configure.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#ifndef __meta__
#include "SkrShaderCompiler/assets/shader_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "067d4b86-f888-4bd7-841c-bc831043e50c")
sattr("serialize" : "json")
SKR_SHADER_COMPILER_API SShaderOptionsImporter final : public SImporter
{
    skr::string jsonPath;

    void* Import(skr::io::RAMService*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sregister_importer();
    
sreflect_struct("guid" : "8c54f8b7-0bf6-4415-ab3b-394a90da7d7f")
SKR_SHADER_COMPILER_API SShaderOptionsCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker("fc9b4a8e-06c7-41e2-a159-f4cf6930ccfc");

sreflect_struct("guid" : "a897c990-abea-4f48-8880-e1ae9a93d777")
sattr("serialize" : "json")
SKR_SHADER_COMPILER_API SShaderImporter final : public SImporter
{
    using shader_options_handle_t = skr::resource::TResourceHandle<skr_shader_options_resource_t>;

    skr::string sourcePath;
    skr::string entry = "main";
    skr::string target;
    eastl::vector<shader_options_handle_t> options_collection;

    void* Import(skr::io::RAMService*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sregister_importer();

sreflect_struct("guid" : "a5cf3ad7-917c-4662-8de9-cd9adbd5eb2a")
SKR_SHADER_COMPILER_API SShaderCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker("1c7d845a-fde8-4487-b1c9-e9c48d6a9867");
} // namespace asset
} // namespace skd