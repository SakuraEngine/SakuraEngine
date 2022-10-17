
#include "asset/config_asset.hpp"
#include "platform/debug.h"
#include "platform/memory.h"
#include "json/reader.h"
%for header in db.headers:
#include "${header}"
%endfor

%for type in db.types:
static struct RegisterImporter${type.id}Helper
{
    RegisterImporter${type.id}Helper()
    {
        using namespace skd::asset;
        auto registry = GetImporterRegistry();
        constexpr skr_guid_t guid = {${type.guidConstant}};
        auto loader = 
            +[](const SAssetRecord* record, simdjson::ondemand::value&& object) -> SImporter*
            {
                auto importer = SkrNew<${type.name}>();
                skr::json::Read(std::move(object), *importer);
                return importer;
            };
        registry->loaders.insert(std::make_pair(guid, loader));
    }
} _RegisterImporter${type.id}Helper;
%endfor