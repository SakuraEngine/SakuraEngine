#pragma once
#include "SkrTool/module.configure.h"
#include <EASTL/vector.h>
#include "simdjson.h"
#include "platform/configure.h"
#include "utils/types.h"
#include "utils/hashmap.hpp"
#include "json/reader.h"
#include "resource/resource_header.h"
#include "asset/cooker.hpp"
#ifndef __meta__
    #include "SkrTool/asset/importer.generated.h"
#endif
namespace skr::io
{
class RAMService;
}

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
TOOL_API SImporter
{
    virtual ~SImporter() {}
    virtual void* Import(skr::io::RAMService*, SCookContext* context) = 0;
    virtual void Destroy(void*) = 0;
    static uint32_t Version() { return UINT32_MAX; }
};

struct TOOL_API SImporterTypeInfo {
    SImporter* (*Load)(const SAssetRecord* record, simdjson::ondemand::value&& object);
    uint32_t (*Version)();
};

struct SImporterRegistry {
    SImporter* LoadImporter(const SAssetRecord* record, simdjson::ondemand::value&& object, skr_guid_t* pGuid = nullptr);
    uint32_t GetImporterVersion(skr_guid_t type);
    skr::flat_hash_map<skr_guid_t, SImporterTypeInfo, skr::guid::hash> loaders;
};

TOOL_API SImporterRegistry* GetImporterRegistry();

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
} // namespace sreflect
} // namespace sreflect