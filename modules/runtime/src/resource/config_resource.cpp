#include "resource/config_resource.h"
#include "EASTL/vector.h"
#include "platform/configure.h"
#include "platform/debug.h"
#include "platform/memory.h"
#include "platform/vfs.h"
#include "resource/resource_factory.h"
#include "resource/resource_header.hpp"
#include "type/type_registry.h"
#include "utils/defer.hpp"
#include "resource/resource_system.h"

skr_config_resource_t::~skr_config_resource_t()
{
    if(configType == skr_type_id_t{})
        return;
    auto type = skr_get_type(&configType);
    type->Destruct(configData);
    type->Free(configData);
}

namespace skr::binary
{
    int WriteHelper<const skr_config_resource_t&>::Write(skr_binary_writer_t *archive, const skr_config_resource_t &value)
    {
        if(auto result = skr::binary::Write(archive, value.configType); result != 0)
            return result;
        if(value.configType == skr_type_id_t{})
            return 0;
        auto type = skr_get_type(&value.configType);
        return type->Serialize(value.configData, archive);
    }

    int ReadHelper<skr_config_resource_t>::Read(skr_binary_reader_t *archive, skr_config_resource_t &value)
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
