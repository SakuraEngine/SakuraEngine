
#include "asset/config_asset.hpp"
#include "platform/debug.h"
#include "platform/memory.h"
#include "json/reader.h"
%for header in db.headers:
#include "${header}"
%endfor

%for type in db.types:
static struct RegisterConfigAsset${type.id}Helper
{
    RegisterConfigAsset${type.id}Helper()
    {
        using namespace skd::asset;
        auto registry = GetConfigRegistry();
        constexpr skr_guid_t guid = {${type.guidConstant}};
        SConfigTypeInfo typeInfo {
            +[](simdjson::ondemand::value&& json, void* address)
            {
                skr::json::Read(std::move(json), *(${type.name}*)address);
            }
        };
        registry->typeInfos.insert(std::make_pair(guid, typeInfo));
    }
} _RegisterConfigAsset${type.id}Helper;
%endfor