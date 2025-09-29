#pragma once
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/cook_system/cooker.hpp"
#include "SkrTextureCompiler/texture_sampler_asset.generated.h" // IWYU pragma: export

namespace skr
{
struct [[sattr(
    guid = "d2fc798b-af43-4865-b953-abba2b6d524a"
    serde = @enable
)]] SKR_TEXTURE_COMPILER_API TextureSamplerImporter final : public Importer
{
    skr::String jsonPath;

    void* Import(skr::io::IRAMService*, CookContext* context) override;
    void Destroy(void* resource) override;
};

struct [[sattr(
    guid = "f06d5542-4c20-48e4-819a-16a6ae13b295"
)]] SKR_TEXTURE_COMPILER_API TextureSamplerCooker final : public Cooker
{
    bool Cook(CookContext* ctx) override;
    uint32_t Version() override { return 1u; }
};
} // namespace skr