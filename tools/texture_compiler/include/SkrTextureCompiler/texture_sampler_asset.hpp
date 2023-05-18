#pragma once
#include "SkrTextureCompiler/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "platform/configure.h"
#include "platform/guid.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#ifndef __meta__
#include "SkrTextureCompiler/texture_sampler_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "d2fc798b-af43-4865-b953-abba2b6d524a")
sattr("serialize" : "json")
SKR_TEXTURE_COMPILER_API STextureSamplerImporter final : public SImporter
{
    skr::string jsonPath;

    void* Import(skr_io_ram_service_t*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sregister_importer();
    
sreflect_struct("guid" : "f06d5542-4c20-48e4-819a-16a6ae13b295")
SKR_TEXTURE_COMPILER_API STextureSamplerCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override { return 1u; }
}
sregister_default_cooker(u8"ab483a53-5024-48f2-87a7-9502063c97f3");
} // namespace asset
} // namespace skd