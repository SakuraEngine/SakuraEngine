
#include "resource/config_resource.h"
#include "platform/debug.h"
#include "serialize.generated.h"
%for header in db.headers:
#include "${header}"
%endfor

%for type in db.types:
static struct RegisterConfigResource${type.id}Helper
{
    RegisterConfigResource${type.id}Helper()
    {
        auto registry = skr::resource::GetConfigRegistry();
        constexpr skr_guid_t guid = {${type.guidConstant}};
        skr::resource::SConfigTypeInfo typeInfo {
            +[](void* address, skr::resource::SBinaryArchive& archive)
            {
                if(archive.serializer)
                    bitsery::serialize(*archive.serializer, *(${type.name}*)address);
                else if(archive.deserializer)
                    bitsery::serialize(*archive.deserializer, *(${type.name}*)address);
                else
                    SKR_UNREACHABLE_CODE();
            }
        };
        registry->typeInfos.insert(std::make_pair(guid, typeInfo));
    }
} _RegisterConfigResource${type.id}Helper;
%endfor