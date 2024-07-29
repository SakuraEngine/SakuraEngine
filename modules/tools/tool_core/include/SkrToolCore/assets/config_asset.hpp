#pragma once
#include "SkrBase/config.h"
#include "SkrToolCore/asset/importer.hpp"
#ifndef __meta__
    #include "SkrToolCore/assets/config_asset.generated.h" // IWYU pragma: export
#endif

namespace skd
{
namespace asset
{
sreflect_struct("guid" : "D5970221-1A6B-42C4-B604-DA0559E048D6")
sattr("serde" : "json")
TOOL_CORE_API SJsonConfigImporter final : public SImporter {
    skr::String assetPath;
    skr_guid_t  configType;
    void*       Import(skr_io_ram_service_t*, SCookContext* context) override;
    void        Destroy(void* resource) override;
};

sreflect_struct("guid" : "EC5275CA-E406-4051-9403-77517C421890")
TOOL_CORE_API SConfigCooker final : public SCooker {
    bool     Cook(SCookContext* ctx) override;
    uint32_t Version() override;
};
} // namespace asset
} // namespace skd