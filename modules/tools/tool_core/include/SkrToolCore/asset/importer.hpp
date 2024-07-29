#pragma once
#include "SkrBase/types.h"
#include "SkrRT/resource/resource_header.hpp"
#include "SkrToolCore/fwd_types.hpp"
#include "SkrToolCore/asset/cooker.hpp"
#ifndef __meta__
    #include "SkrToolCore/asset/importer.generated.h" // IWYU pragma: export
#endif

namespace skd::asset
{
using namespace skr;
template <class T>
void          RegisterImporter(skr_guid_t guid);

sreflect_struct("guid" : "76044661-E2C9-43A7-A4DE-AEDD8FB5C847", "serde" : "json")
TOOL_CORE_API SImporter {
    static constexpr uint32_t kDevelopmentVersion = UINT32_MAX;

    virtual ~SImporter()                                                 = default;
    virtual void*   Import(skr_io_ram_service_t*, SCookContext* context) = 0;
    virtual void    Destroy(void*)                                       = 0;
    static uint32_t Version() { return kDevelopmentVersion; }
};

struct TOOL_CORE_API SImporterTypeInfo {
    SImporter* (*Load)(const SAssetRecord* record, skr::archive::JsonReader* object);
    uint32_t (*Version)();
};

struct SImporterRegistry {
    virtual SImporter* LoadImporter(const SAssetRecord* record, skr::archive::JsonReader* object, skr_guid_t* pGuid = nullptr) = 0;
    virtual uint32_t   GetImporterVersion(skr_guid_t type)                                                                     = 0;
    virtual void       RegisterImporter(skr_guid_t type, SImporterTypeInfo info)                                               = 0;
};

TOOL_CORE_API SImporterRegistry* GetImporterRegistry();
} // namespace skd::asset

template <class T>
void skd::asset::RegisterImporter(skr_guid_t guid)
{
    auto registry = GetImporterRegistry();
    auto loader =
    +[](const SAssetRecord* record, skr::archive::JsonReader* object) -> SImporter* {
        auto importer = SkrNew<T>();
        skr::json_read(object, *importer);
        return importer;
    };
    SImporterTypeInfo info{ loader, T::Version };
    registry->RegisterImporter(guid, info);
}