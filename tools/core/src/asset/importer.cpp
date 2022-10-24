#include "asset/cooker.hpp"
#include "asset/importer.hpp"
#include "utils/types.h"
#include "json/reader.h"

namespace skd::asset
{
SImporterRegistry* GetImporterRegistry()
{
    static SImporterRegistry registry;
    return &registry;
}

SImporter* SImporterRegistry::LoadImporter(const SAssetRecord* record, simdjson::ondemand::value&& object)
{
    skr_guid_t type;
    skr::json::Read(object["importerType"].value_unsafe(), type);
    auto iter = loaders.find(type);
    if (iter != loaders.end())
        return iter->second.Load(record, std::move(object));
    return nullptr;
}
uint32_t SImporterRegistry::GetImporterVersion(skr_guid_t type)
{
    auto iter = loaders.find(type);
    if (iter != loaders.end())
        return iter->second.Version();
    return UINT32_MAX;
}
} // namespace skd::asset