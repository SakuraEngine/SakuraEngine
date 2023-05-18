#pragma once
#include "SkrToolCore/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "misc/types.h"
#include "platform/guid.hpp"
#ifndef __meta__
#include "SkrToolCore/assets/config_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "D5970221-1A6B-42C4-B604-DA0559E048D6")
TOOL_CORE_API SJsonConfigImporter final : public SImporter
{
    skr::string assetPath;
    skr_guid_t configType;
    void* Import(skr_io_ram_service_t*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sattr("serialize" : "json")
sregister_importer();


sreflect_struct("guid" : "EC5275CA-E406-4051-9403-77517C421890")
TOOL_CORE_API SConfigCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_default_cooker(u8"8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74");
} // namespace asset
} // namespace skd