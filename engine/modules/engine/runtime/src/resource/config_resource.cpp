#include "SkrRuntime/resource/config_resource.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrCore/memory/memory.h"
#include "SkrRTTR/type_registry.hpp"
#include "SkrRTTR/type.hpp"
#include <SkrBase/type_info.hpp>
#include "SkrRTTR/export/extern_methods.hpp"

skr_config_resource_t::~skr_config_resource_t()
{
    if (configType.is_zero())
        return;
    auto type = skr::get_type_from_guid(configType);
    type->find_default_ctor().invoke(configData);
    sakura_free_aligned(configData, type->alignment());
}

void skr_config_resource_t::SetType(skr::GUID type)
{
    if (!(configType.is_zero()))
    {
        SKR_ASSERT(configData);
        auto oldType = skr::get_type_from_guid(configType);
        oldType->invoke_dtor(configData);
        sakura_free_aligned(configData, oldType->alignment());
    }
    configType = type;
    auto newType = skr::get_type_from_guid(configType);
    configData = sakura_malloc_aligned(newType->size(), newType->alignment());
    newType->find_default_ctor().invoke(configData);
}

namespace skr
{
void Serialize<skr_config_resource_t>::read(ArchiveRead& r, skr_config_resource_t& v)
{
    Archive::ObjectScope obj_scope(r);
    SKR_ASSERT(obj_scope.is_success());

    // read type
    SKR_FAST_CHECK(r.key_value(u8"config_type", v.configType), );
    SKR_FAST_CHECK(!v.configType.is_zero(), );

    // find type
    auto* type = skr::get_type_from_guid(v.configType);
    SKR_FAST_CHECK(type != nullptr, );

    // alloc config data
    v.configData = type->alloc();

    // construct config data
    auto found_ctor = type->find_default_ctor();
    SKR_FAST_CHECK(found_ctor.is_valid(), );
    found_ctor.invoke(v.configData);

    // do serialize
    auto found_serde_read = type->find_serde_read();
    SKR_FAST_CHECK(found_serde_read.is_valid(), );
    found_serde_read.invoke(r, v.configData);
}
void Serialize<skr_config_resource_t>::write(ArchiveWrite& w, const skr_config_resource_t& v)
{
    Archive::ObjectScope obj_scope(w);
    SKR_ASSERT(obj_scope.is_success());

    // write type
    SKR_FAST_CHECK(w.key_value(u8"config_type", v.configType), );

    // find type
    auto* type = skr::get_type_from_guid(v.configType);
    SKR_FAST_CHECK(type != nullptr, );

    // do serialize
    auto found_serde_write = type->find_serde_write();
    SKR_FAST_CHECK(found_serde_write.is_valid(), );
    found_serde_write.invoke(w, v.configData);
}

skr::GUID ConfigFactory::GetResourceType() { return skr::type_id_of<skr_config_resource_t>(); }
} // namespace skr
