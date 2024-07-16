#include "SkrRT/resource/config_resource.h"
#include "SkrBase/misc/debug.h"
#include "SkrCore/memory/memory.h"
#include "SkrRTTR/type_registry.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

skr_config_resource_t::~skr_config_resource_t()
{
    if (configType == skr_guid_t{})
        return;
    auto type = skr::rttr::get_type_from_guid(configType);
    // type->call_dtor(configData); // TODO. resume rttr
    sakura_free_aligned(configData, type->alignment());
}

void skr_config_resource_t::SetType(skr_guid_t type)
{
    if (!(configType == skr_guid_t{}))
    {
        SKR_ASSERT(configData);
        auto oldType = skr::rttr::get_type_from_guid(configType);
        // oldType->call_dtor(configData); // TODO. resume rttr
        sakura_free_aligned(configData, oldType->alignment());
    }
    configType   = type;
    auto newType = skr::rttr::get_type_from_guid(configType);
    configData   = sakura_malloc_aligned(newType->size(), newType->alignment());
    // newType->call_ctor(configData); // TODO. resume rttr
}

namespace skr
{
bool BinSerde<skr_config_resource_t>::read(SBinaryReader* r, skr_config_resource_t& v)
{
    if (!bin_read(r, v.configType))
        return false;
    if (v.configType == skr_guid_t{})
        return true;
    auto type    = skr::rttr::get_type_from_guid(v.configType);
    v.configData = sakura_malloc_aligned(type->size(), type->alignment());
    // return type->read_binary(value.configData, archive); // TODO. resume rttr
    SKR_UNIMPLEMENTED_FUNCTION();
    return {};
}
bool BinSerde<skr_config_resource_t>::write(SBinaryWriter* w, const skr_config_resource_t& v)
{
    if (!bin_write(w, v.configType))
        return false;
    if (v.configType == skr_guid_t{})
        return true;
    // auto type = skr::rttr::get_type_from_guid(value.configType);
    // return type->write_binary(value.configData, archive); // TODO. resume rttr
    SKR_UNIMPLEMENTED_FUNCTION();
    return {};
}
} // namespace skr

namespace skr
{
namespace resource
{
skr_guid_t SConfigFactory::GetResourceType() { return skr::rttr::type_id_of<skr_config_resource_t>(); }
} // namespace resource
} // namespace skr
