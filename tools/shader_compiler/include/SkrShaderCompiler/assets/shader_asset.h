#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "platform/configure.h"

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "a897c990-abea-4f48-8880-e1ae9a93d777")
sattr("serialize" : "json")
SKR_SHADER_COMPILER_API SShaderImporter final : public SImporter
{
    eastl::string assetPath;
    eastl::string entry = "main";
    eastl::string target;

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