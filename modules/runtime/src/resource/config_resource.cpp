#include "resource/config_resource.h"
#include "EASTL/vector.h"
#include "platform/configure.h"
#include "platform/debug.h"
#include "platform/guid.hpp"
#include "platform/memory.h"
#include "platform/vfs.h"
#include "resource/resource_factory.h"
#include "resource/resource_header.hpp"
#include "type/type.hpp"
#include "utils/defer.hpp"
#include "resource/resource_system.h"
#include "serde/binary/reader.h"
#include "serde/binary/writer.h"

skr_config_resource_t::~skr_config_resource_t()
{
    if(configType == skr_type_id_t{})
        return;
    auto type = skr_get_type(&configType);
    type->Destruct(configData);
    type->Free(configData);
}

void skr_config_resource_t::SetType(skr_type_id_t type)
{
    if(!(configType == skr_type_id_t{}))
    {
        SKR_ASSERT(configData);
        auto oldType = skr_get_type(&configType);
        oldType->Destruct(configData);
        oldType->Free(configData);
    }
    configType = type;
    auto newType = skr_get_type(&configType);
    configData = newType->Malloc();
    newType->Construct(configData, nullptr, 0);
}

namespace skr::binary
{
    int WriteTrait<const skr_config_resource_t&>::Write(skr_binary_writer_t *archive, const skr_config_resource_t &value)
    {
        if(auto result = skr::binary::Write(archive, value.configType); result != 0)
            return result;
        if(value.configType == skr_type_id_t{})
            return 0;
        auto type = skr_get_type(&value.configType);
        return type->Serialize(value.configData, archive);
    }

    int ReadTrait<skr_config_resource_t>::Read(skr_binary_reader_t *archive, skr_config_resource_t &value)
    {
        if(auto result = skr::binary::Read(archive, value.configType); result != 0)
            return result;
        if(value.configType == skr_type_id_t{})
            return 0;
        auto type = skr_get_type(&value.configType);
        value.configData = type->Malloc();
        return type->Deserialize(value.configData, archive);
    }
}

namespace skr
{
namespace resource
{
skr_guid_t SConfigFactory::GetResourceType() { return get_type_id_skr_config_resource_t(); }
} // namespace resource
} // namespace skr
