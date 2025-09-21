#pragma once
#include "SkrContainersDef/string.hpp"
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/cook_system/cooker.hpp"
#include "SkrToolCore/assets/config_asset.generated.h" // IWYU pragma: export

namespace skd::asset
{
struct [[sattr(
    guid = "D5970221-1A6B-42C4-B604-DA0559E048D6"
    serde = @enable
)]] TOOL_CORE_API JsonConfigImporter final : public Importer
{
    skr::String assetPath;
    GUID configType;
    void* Import(skr::io::IRAMService*, CookContext* context) override;
    void Destroy(void* resource) override;
};

struct [[sattr(
    guid = "EC5275CA-E406-4051-9403-77517C421890"
)]] TOOL_CORE_API ConfigCooker final : public Cooker
{
    bool Cook(CookContext* ctx) override;
    uint32_t Version() override;
};
} // namespace skd::asset