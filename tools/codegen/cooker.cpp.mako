
#include "asset/cooker.hpp"
#include "platform/debug.h"
#include "platform/memory.h"
#include "json/reader.h"
%for header in db.headers:
#include "${header}"
%endfor

%for type in db.types:
static struct RegisterCooker${type.id}Helper
{
    RegisterCooker${type.id}Helper()
    {
        using namespace skd::asset;
        static constexpr skr_guid_t guid = {${type.guidConstant}};
        static ${type.name} instance;
        GetCookSystem()->cookers.insert(std::make_pair(guid, &instance));
    }
} _RegisterCooker${type.id}Helper;
%endfor