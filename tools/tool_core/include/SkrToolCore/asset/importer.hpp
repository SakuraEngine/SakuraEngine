#pragma once
#include "SkrToolCore/module.configure.h"
#include "json/reader.h"
#include "resource/resource_header.h"
#include "SkrToolCore/asset/cooker.hpp"
#ifndef __meta__
    #include "SkrToolCore/asset/importer.generated.h"
#endif

namespace skr::io { class RAMService; }

namespace skd sreflect
{
using namespace skr;
namespace asset sreflect
{

template<class T>
void RegisterImporter(skr_guid_t guid);
#define sregister_importer() sstatic_ctor(skd::asset::RegisterImporter<$T>($guid));

struct sreflect sattr(
    "guid" : "76044661-E2C9-43A7-A4DE-AEDD8FB5C847", 
    "serialize" : "json"
)
TOOL_CORE_API SImporter
{
    static constexpr uint32_t kDevelopmentVersion = UINT32_MAX;

    virtual ~SImporter() = default;
    virtual void* Import(skr::io::RAMService*, SCookContext* context) = 0;
    virtual void Destroy(void*) = 0;
    static uint32_t Version() { return kDevelopmentVersion; }
};

struct TOOL_CORE_API SImporterTypeInfo {
    SImporter* (*Load)(const SAssetRecord* record, simdjson::ondemand::value&& object);
    uint32_t (*Version)();
};

struct SImporterRegistry {
    SImporter* LoadImporter(const SAssetRecord* record, simdjson::ondemand::value&& object, skr_guid_t* pGuid = nullptr);
    uint32_t GetImporterVersion(skr_guid_t type);
    skr::flat_hash_map<skr_guid_t, SImporterTypeInfo, skr::guid::hash> loaders;
};

TOOL_CORE_API SImporterRegistry* GetImporterRegistry();

struct SImporterFactory {
    virtual bool CanImport(const SAssetRecord* record) = 0;
    virtual skr_guid_t GetResourceType() = 0;
    virtual void CreateImporter(const SAssetRecord* record) = 0;
};

template<class T>
void RegisterImporter(skr_guid_t guid)
{
    auto registry = GetImporterRegistry();
    auto loader = 
        +[](const SAssetRecord* record, simdjson::ondemand::value&& object) -> SImporter*
        {
            auto importer = SkrNew<T>();
            skr::json::Read(std::move(object), *importer);
            return importer;
        };
    SImporterTypeInfo info { loader, T::Version };
    registry->loaders.insert(std::make_pair(guid, info));
}
} // namespace asset
} // namespace skd