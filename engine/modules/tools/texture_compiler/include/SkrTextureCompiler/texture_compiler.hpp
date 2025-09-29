#pragma once
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/cook_system/cooker.hpp"
#include "SkrContainersDef/string.hpp"
#include "SkrTextureCompiler/texture_compiler.generated.h" // IWYU pragma: export

namespace skr
{
struct [[sattr(
    guid = "a26c2436-9e5f-43c4-b4d7-e5373d353bae"
    serde = @enable
)]] SKR_TEXTURE_COMPILER_API TextureImporter final : public Importer
{
    skr::String assetPath;
    uint32_t mip_count = 1;

    void* Import(skr::io::IRAMService*, CookContext* context) override;
    void Destroy(void* resource) override;
};

struct [[sattr(
    guid = "F9B45BF9-3767-4B40-B0B3-D4BBC228BCEC"
)]] SKR_TEXTURE_COMPILER_API TextureCooker final : public Cooker
{
    bool Cook(CookContext* ctx) override;
    uint32_t Version() override;
};

} // namespace skr