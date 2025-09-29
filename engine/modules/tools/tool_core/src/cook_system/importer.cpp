#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrContainers/hashmap.hpp"

namespace skr
{
Importer::Importer()
{
}

struct ImporterRegistryImpl : public ImporterRegistry
{
    skr::RC<Importer> LoadImporter(skr::ArReadJson* json) override;
    void StoreImporter(skr::ArWriteJson* writer, skr::RC<Importer> importer) override;
    uint32_t GetImporterVersion(skr::GUID type) override;
    void RegisterImporter(skr::GUID type, ImporterTypeInfo info) override;

    skr::FlatHashMap<skr::GUID, ImporterTypeInfo, skr::Hash<skr::GUID>> importer_types;
};

ImporterRegistry* GetImporterRegistry()
{
    static ImporterRegistryImpl registry;
    return &registry;
}

skr::RC<Importer> ImporterRegistryImpl::LoadImporter(skr::ArReadJson* json)
{
    Archive::ObjectScope obj_scope(*json);
    SKR_FAST_CHECK(obj_scope.is_success(), nullptr);

    skr::GUID type;
    SKR_FAST_CHECK(json->key_value(u8"importer_type", type), nullptr);

    // find importer type
    auto iter = importer_types.find(type);
    if (iter != importer_types.end())
    {
        auto importer = iter->second.Create();
        // read importer data
        SKR_FAST_CHECK(json->key(u8"importer"), nullptr);
        iter->second.Load(json, importer);
        SKR_FAST_CHECK(json->checkpoint(), nullptr);
    }
    return nullptr;
}

void ImporterRegistryImpl::StoreImporter(skr::ArWriteJson* writer, skr::RC<Importer> importer)
{
    auto iter = importer_types.find(importer->GetType());
    if (iter != importer_types.end())
    {
        iter->second.Store(writer, importer);
    }
}

uint32_t ImporterRegistryImpl::GetImporterVersion(skr::GUID type)
{
    auto iter = importer_types.find(type);
    if (iter != importer_types.end())
        return iter->second.Version();
    return UINT32_MAX;
}

void ImporterRegistryImpl::RegisterImporter(skr::GUID type, ImporterTypeInfo info)
{
    importer_types.insert({ type, info });
}
} // namespace skr
