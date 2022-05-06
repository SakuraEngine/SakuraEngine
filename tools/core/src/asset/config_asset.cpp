#include "asset/config_asset.hpp"
#include "platform/configure.h"
#include "platform/memory.h"
#include "resource/config_resource.h"
#include "json/reader.h"
#include "platform/debug.h"
#include "simdjson.h"
#include "utils/log.h"
#include "utils/defer.hpp"
namespace skd
{
namespace asset
{
void* SJsonConfigImporter::Import(const SAssetRecord* record)
{
    auto registry = GetConfigRegistry();
    auto iter = registry->typeInfos.find(configType);
    if (iter == registry->typeInfos.end())
    {
        SKR_LOG_ERROR("import resource %s failed, type is not registered as config", record->path.u8string().c_str());
        return nullptr;
    }
    auto file = fopen(record->path.u8string().c_str(), "r");
    fseek(file, 0, SEEK_END);
    auto size = ftell(file);
    rewind(file);
    char* buffer = (char*)sakura_malloc(size + 1);
    SKR_DEFER({fclose(file); sakura_free(buffer); });
    size = fread(buffer, 1, size, file);
    buffer[size] = 0;
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(buffer, size);
    return iter->second.Import(doc.value()); //导入具体数据
}

bool SJsonConfigImporterFactory::CanImport(const SAssetRecord* record)
{
    if (record->path.extension() == ".json")
        return true;
}
skr_guid_t SJsonConfigImporterFactory::GetResourceType()
{
    return get_type_id_skr_config_resource_t();
}
SImporter* SJsonConfigImporterFactory::CreateImporter(const SAssetRecord* record)
{
    // TODO: invoke user interface?
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
SImporter* SJsonConfigImporterFactory::LoadImporter(const SAssetRecord* record, simdjson::ondemand::value&& object)
{
    auto importer = SkrNew<SJsonConfigImporter>(record->guid);
    skr::json::Read(std::move(object), *importer); //读取导入器本身的数据
    return importer;
}
} // namespace asset
} // namespace skd