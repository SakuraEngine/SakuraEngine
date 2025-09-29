#pragma once
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/cook_system/cooker.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrShaderCompiler/assets/shader_asset.generated.h" // IWYU pragma: export

namespace skr
{
struct [[sattr(
    guid = "067d4b86-f888-4bd7-841c-bc831043e50c"
    serde = @enable
)]] SKR_SHADER_COMPILER_API ShaderOptionImporter final : public Importer
{
    skr::String jsonPath;

    void* Import(skr::io::IRAMService*, CookContext* context) override;
    void Destroy(void* resource) override;
};

struct [[sattr(
    guid = "8c54f8b7-0bf6-4415-ab3b-394a90da7d7f"
)]] SKR_SHADER_COMPILER_API ShaderOptionsCooker final : public Cooker
{
    bool Cook(CookContext* ctx) override;
    uint32_t Version() override;
};

struct [[sattr(
    guid = "a897c990-abea-4f48-8880-e1ae9a93d777"
    serde = @enable
)]] SKR_SHADER_COMPILER_API ShaderImporter final : public Importer
{
    using shader_options_handle_t = skr::AsyncResource<skr::ShaderOptionsResource>;

    skr::String sourcePath;
    skr::String entry = u8"main";
    skr::String target;

    skr::Vector<shader_options_handle_t> switch_assets;
    skr::Vector<shader_options_handle_t> option_assets;

    void* Import(skr::io::IRAMService*, CookContext* context) override;
    void Destroy(void* resource) override;
};

struct [[sattr(
    guid = "a5cf3ad7-917c-4662-8de9-cd9adbd5eb2a"
)]] SKR_SHADER_COMPILER_API ShaderCooker final : public Cooker
{
    bool Cook(CookContext* ctx) override;
    uint32_t Version() override;
};
} // namespace skr