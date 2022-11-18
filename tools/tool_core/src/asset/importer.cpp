#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "utils/types.h"
#include "platform/guid.hpp"
#include "json/reader.h"

namespace skd::asset
{
    
struct SImporterRegistryImpl : public SImporterRegistry 
{
    SImporter* LoadImporter(const SAssetRecord* record, simdjson::ondemand::value&& object, skr_guid_t* pGuid = nullptr) override;
    uint32_t GetImporterVersion(skr_guid_t type) override;
    void RegisterImporter(skr_guid_t type, SImporterTypeInfo info) override;

    skr::flat_hash_map<skr_guid_t, SImporterTypeInfo, skr::guid::hash> loaders;
};

SImporterRegistry* GetImporterRegistry()
{
    static SImporterRegistryImpl registry;
    return &registry;
}

SImporter* SImporterRegistryImpl::LoadImporter(const SAssetRecord* record, simdjson::ondemand::value&& object, skr_guid_t* pGuid)
{
    skr_guid_t type;
    skr::json::Read(object["importerType"].value_unsafe(), type);
    if (pGuid) *pGuid = type;
    auto iter = loaders.find(type);
    if (iter != loaders.end())
        return iter->second.Load(record, std::move(object));
    return nullptr;
}
uint32_t SImporterRegistryImpl::GetImporterVersion(skr_guid_t type)
{
    auto iter = loaders.find(type);
    if (iter != loaders.end())
        return iter->second.Version();
    return UINT32_MAX;
}

void SImporterRegistryImpl::RegisterImporter(skr_guid_t type, SImporterTypeInfo info)
{
    loaders.insert({type, info});
}
} // namespace skd::asset
