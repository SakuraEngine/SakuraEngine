#include "SkrRT/resource/config_resource.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/guid.hpp"
#include "SkrMemory/memory.h"
#include "SkrRT/serde/binary/reader.h"
#include "SkrRT/serde/binary/writer.h"
#include "SkrRT/rttr/type_registry.hpp"
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

skr_config_resource_t::~skr_config_resource_t()
{
    if (configType == skr_guid_t{})
        return;
    auto type = skr::rttr::get_type_from_guid(configType);
    type->call_dtor(configData);
    sakura_free_aligned(configData, type->alignment());
}

void skr_config_resource_t::SetType(skr_guid_t type)
{
    if (!(configType == skr_guid_t{}))
    {
        SKR_ASSERT(configData);
        auto oldType = skr::rttr::get_type_from_guid(configType);
        oldType->call_dtor(configData);
        sakura_free_aligned(configData, oldType->alignment());
    }
    configType   = type;
    auto newType = skr::rttr::get_type_from_guid(configType);
    configData   = sakura_malloc_aligned(newType->size(), newType->alignment());
    newType->call_ctor(configData);
}

namespace skr::binary
{
int WriteTrait<skr_config_resource_t>::Write(skr_binary_writer_t* archive, const skr_config_resource_t& value)
{
    if (auto result = skr::binary::Write(archive, value.configType); result != 0)
        return result;
    if (value.configType == skr_guid_t{})
        return 0;
    auto type = skr::rttr::get_type_from_guid(value.configType);
    return type->write_binary(value.configData, archive);
}

int ReadTrait<skr_config_resource_t>::Read(skr_binary_reader_t* archive, skr_config_resource_t& value)
{
    if (auto result = skr::binary::Read(archive, value.configType); result != 0)
        return result;
    if (value.configType == skr_guid_t{})
        return 0;
    auto type        = skr::rttr::get_type_from_guid(value.configType);
    value.configData = sakura_malloc_aligned(type->size(), type->alignment());
    return type->read_binary(value.configData, archive);
}
} // namespace skr::binary

namespace skr
{
namespace resource
{
skr_guid_t SConfigFactory::GetResourceType() { return skr::rttr::type_id<skr_config_resource_t>(); }
} // namespace resource
} // namespace skr
