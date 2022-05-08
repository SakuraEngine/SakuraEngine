#include "asset/asset_registry.hpp"
#include "ghc/filesystem.hpp"
#include "platform/debug.h"
#include "platform/guid.h"
#include "simdjson.h"
#include "utils/log.h"
#include "platform/memory.h"
#include "utils/defer.hpp"
#include "json/reader.h"

namespace skd::asset
{
TOOL_API SAssetRegistry* GetAssetRegistry()
{
    static SAssetRegistry registry;
    return &registry;
}
} // namespace skd::asset

namespace skd::asset
{
SAssetRecord* SAssetRegistry::ImportAsset(ghc::filesystem::path path)
{
    auto metaPath = path;
    metaPath.replace_extension(".meta");
    if (!ghc::filesystem::exists(metaPath))
    {
        SKR_LOG_ERROR("[SAssetRegistry::ImportAsset] meta file %s not exist", path.u8string().c_str());
        return nullptr;
    }
    auto record = SkrNew<SAssetRecord>();
    // TODO: replace file load with skr api
    record->meta = simdjson::padded_string::load(metaPath.u8string());
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(record->meta);
    skr_guid_t guid;
    skr::json::Read(doc.find_field("guid").value(), guid);
    record->guid = guid;
    record->path = path;
    assets.insert(std::make_pair(guid, record));
    return record;
}
SAssetRecord* SAssetRegistry::GetAssetRecord(const skr_guid_t& guid)
{
    auto iter = assets.find(guid);
    return iter != assets.end() ? iter->second : nullptr;
}
void* SAssetRegistry::ImportResource(const skr_guid_t& guid, skr_guid_t& resourceType)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
SAssetRegistry::~SAssetRegistry()
{
    for (auto& pair : assets)
        SkrDelete(pair.second);
}
} // namespace skd::asset