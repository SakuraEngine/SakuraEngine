#include "asset/importer.hpp"
#include "platform/guid.h"
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
    skr::json::Read(object.find_field("importerType").value_unsafe(), type);
    auto iter = loaders.find(type);
    if (iter != loaders.end())
        return iter->second(record, std::move(object));
    return nullptr;
}
} // namespace skd::asset