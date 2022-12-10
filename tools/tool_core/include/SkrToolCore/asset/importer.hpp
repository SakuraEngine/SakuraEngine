#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "resource/resource_header.hpp"
#include "SkrToolCore/asset/cooker.hpp"
#include "json/reader_fwd.h"
#ifndef __meta__
    #include "SkrToolCore/asset/importer.generated.h"
#endif

namespace skd sreflect
{
using namespace skr;
namespace asset sreflect
{
template<class T>
void RegisterImporter(skr_guid_t guid);

sreflect_struct("guid" : "76044661-E2C9-43A7-A4DE-AEDD8FB5C847", "serialize" : "json")
TOOL_CORE_API SImporter
{
    static constexpr uint32_t kDevelopmentVersion = UINT32_MAX;

    virtual ~SImporter() = default;
    virtual void* Import(skr_io_ram_service_t*, SCookContext* context) = 0;
    virtual void Destroy(void*) = 0;
    static uint32_t Version() { return kDevelopmentVersion; }
};

struct TOOL_CORE_API SImporterTypeInfo {
    SImporter* (*Load)(const SAssetRecord* record, skr::json::value_t&& object);
    uint32_t (*Version)();
};

struct SImporterRegistry {
    virtual SImporter* LoadImporter(const SAssetRecord* record, skr::json::value_t&& object, skr_guid_t* pGuid = nullptr) = 0;
    virtual uint32_t GetImporterVersion(skr_guid_t type) = 0;
    virtual void RegisterImporter(skr_guid_t type, SImporterTypeInfo info) = 0;
};

TOOL_CORE_API SImporterRegistry* GetImporterRegistry();
} // namespace asset
} // namespace skd

namespace skr::json { 
    template <class T> error_code Read(skr::json::value_t&& json, T& value);
} // namespace skr::json

template<class T>
void skd::asset::RegisterImporter(skr_guid_t guid)
{
    auto registry = GetImporterRegistry();
    auto loader = 
        +[](const SAssetRecord* record, skr::json::value_t&& object) -> SImporter*
        {
            auto importer = SkrNew<T>();
            skr::json::Read(std::move(object), *importer);
            return importer;
        };
    SImporterTypeInfo info { loader, T::Version };
    registry->RegisterImporter(guid, info);
}
#define sregister_importer() sstatic_ctor(skd::asset::RegisterImporter<$T>($guid));