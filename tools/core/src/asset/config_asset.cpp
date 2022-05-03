#include "asset/config_asset.hpp"
#include "platform/configure.h"
#include "platform/memory.h"
#include "resource/config_resource.h"
#include "json/reader.h"
#include "platform/debug.h"
namespace skd
{
namespace asset
{
bool SJsonConfigImporterFactory::CanImport(const SAssetRecord* record)
{
    // TODO: Path utils
    //  if(record->path.get_extension() == ".config")
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
    skr::json::Read(std::move(object), *importer);
    return importer;
}
} // namespace asset
} // namespace skd