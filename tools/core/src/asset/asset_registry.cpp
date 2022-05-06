#include "asset/asset_registry.hpp"
#include "ghc/filesystem.hpp"
#include "platform/debug.h"
#include "platform/guid.h"
#include "utils/log.h"
#include "platform/memory.h"
#include "utils/defer.hpp"
#include "json/reader.h"

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
    auto file = fopen(metaPath.u8string().c_str(), "r");
    fseek(file, 0, SEEK_END);
    auto size = ftell(file);
    rewind(file);
    char* buffer = (char*)sakura_malloc(size + 1);
    SKR_DEFER({fclose(file); sakura_free(buffer); });
    size = fread(buffer, 1, size, file);
    buffer[size] = 0;
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(buffer, size);
    skr_guid_t guid;
    skr::json::Read(doc.find_field("guid").value(), guid);
    auto record = SkrNew<SAssetRecord>();
    record->guid = guid;
    record->path = path;
    record->meta = std::move(doc);
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