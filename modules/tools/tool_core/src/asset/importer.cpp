#include "SkrSerde/json/reader.h"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"

namespace skd::asset
{
struct SImporterRegistryImpl : public SImporterRegistry {
    SImporter* LoadImporter(const SAssetRecord* record, skr::archive::JsonReader* object, skr_guid_t* pGuid = nullptr) override;
    uint32_t   GetImporterVersion(skr_guid_t type) override;
    void       RegisterImporter(skr_guid_t type, SImporterTypeInfo info) override;

    skr::FlatHashMap<skr_guid_t, SImporterTypeInfo, skr::Hash<skr_guid_t>> loaders;
};

SImporterRegistry* GetImporterRegistry()
{
    static SImporterRegistryImpl registry;
    return &registry;
}

SImporter* SImporterRegistryImpl::LoadImporter(const SAssetRecord* record, skr::archive::JsonReader* object, skr_guid_t* pGuid)
{
    skr_guid_t type;
    {
        object->StartObject(); // start importer object
        object->Key(u8"importerType");
        skr::json::Read(object, type);
        object->EndObject();
    }
    if (pGuid) *pGuid = type;
    auto iter = loaders.find(type);
    if (iter != loaders.end())
    {
        object->Key(u8"importer");
        return iter->second.Load(record, object);
    }
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
    loaders.insert({ type, info });
}
} // namespace skd::asset
