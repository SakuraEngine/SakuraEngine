#include "resource/config_resource.h"
#include "bitsery/deserializer.h"
#include "bitsery/details/adapter_common.h"
#include "platform/configure.h"
#include "platform/memory.h"
#include "resource/resource_factory.h"
#include "resource/resource_header.h"
#include "type/type_registry.h"

namespace skr
{
namespace resource
{
skr_guid_t SConfigFactory::GetResourceType() { return get_type_id_skr_config_resource_t(); }
bool SConfigFactory::Deserialize(skr_resource_record_t* record, SBinaryDeserializer& archive)
{
    if (!SResourceFactory::Deserialize(record, archive))
        return false;
    skr_type_id_t typeId;
    archive(typeId);
    auto type = skr_get_type(&typeId);
    auto align = type->Align();
    auto baseSize = sizeof(skr_config_resource_t);
    auto offset = ((baseSize + align - 1) / align) * align;
    auto size = offset + type->Size();
    auto mem = sakura_malloc(size);
    auto res = new (mem) skr_config_resource_t;
    res->configType = typeId;
    res->configData = (char*)mem + offset;
    DeserializeConfig(typeId, res->configData, archive);
    record->resource = res;
    record->destructor = [](void* mem) {
        auto res = (skr_config_resource_t*)(mem);
        auto type = skr_get_type(&res->configType);
        type->Destruct(res->configData);
        SkrDelete(res);
    };
    return archive.adapter().error() == bitsery::ReaderError::NoError;
}

void SConfigFactory::Serialize(const skr_config_resource_t& config, SBinarySerializer& archive)
{
    archive(config.configType);
    SConfigFactory::SerializeConfig(config.configType, config.configData, archive);
}

void SConfigFactory::DeserializeConfig(const skr_type_id_t& id, void* address, SBinaryDeserializer& deserializer)
{
    auto registry = GetConfigRegistry();
    auto iter = registry->typeInfos.find(id);
    SKR_ASSERT(registry->typeInfos.end() != iter);
    SBinaryArchive archive{};
    archive.deserializer = &deserializer;
    iter->second.Serialize(address, archive);
}

void SConfigFactory::SerializeConfig(const skr_type_id_t& id, void* address, SBinarySerializer& serializer)
{
    auto registry = GetConfigRegistry();
    auto iter = registry->typeInfos.find(id);
    SKR_ASSERT(registry->typeInfos.end() != iter);
    SBinaryArchive archive{};
    archive.serializer = &serializer;
    iter->second.Serialize(address, archive);
}
} // namespace resource
} // namespace skr
