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

namespace skr::resource
{
struct RUNTIME_API SConfigRegistryImpl : public SConfigRegistry
{
    virtual void RegisterConfigType(const skr_guid_t& guid, const SConfigTypeInfo& info) override
    {
        typeInfos.insert(std::make_pair(guid, info));
    }
    
    virtual const SConfigTypeInfo* FindConfigType(const skr_guid_t& guid) override
    {
        auto it = typeInfos.find(guid);
        if (it != typeInfos.end())
        {
            return &it->second;
        }
        return nullptr;
    }
    skr::flat_hash_map<skr_guid_t, SConfigTypeInfo, skr::guid::hash> typeInfos;
};

RUNTIME_API SConfigRegistry* GetConfigRegistry()
{
    static SConfigRegistryImpl registry;
    return &registry;
}
} // namespace skr::resource

namespace skr
{
namespace resource
{
skr_guid_t SConfigFactory::GetResourceType() { return get_type_id_skr_config_resource_t(); }

skr_config_resource_t* SConfigFactory::NewConfig(skr_type_id_t& id)
{
    auto type = skr_get_type(&id);
    auto align = type->Align();
    auto baseSize = sizeof(skr_config_resource_t);
    auto offset = ((baseSize + align - 1) / align) * align;
    auto size = offset + type->Size();
    auto res = SkrNewSized<skr_config_resource_t>(size);
    res->configType = id;
    res->configData = (char*)res + offset;
    return res;
}

void SConfigFactory::DestroyConfig(struct skr_config_resource_t* config)
{
    SkrDelete(config);
}

ESkrLoadStatus SConfigFactory::Load(skr_resource_record_t* record)
{
    auto data = record->activeRequest->GetData();
    struct SpanReader
    {
        gsl::span<const uint8_t> data;
        size_t offset = 0;
        int read(void* dst, size_t size)
        {
            if (offset + size > data.size())
                return -1;
            memcpy(dst, data.data() + offset, size);
            offset += size;
            return 0;
        }
    } reader = { data };
    skr_binary_reader_t archive{reader};
    if (Deserialize(record, archive))
        return SKR_LOAD_STATUS_SUCCEED;
    else
        return SKR_LOAD_STATUS_FAILED;
}

bool SConfigFactory::Deserialize(skr_resource_record_t* record, skr_binary_reader_t& archive)
{
    namespace bin = skr::binary;
    if(bin::Archive(&archive, record->header) != 0)
        return false;
    skr_type_id_t typeId;
    bin::Archive(&archive, typeId);
    auto res = NewConfig(typeId);
    DeserializeConfig(typeId, res->configData, archive);
    record->resource = res;
    record->destructor = [](void* mem) {
        auto res = (skr_config_resource_t*)(mem);
        auto type = skr_get_type(&res->configType);
        type->Destruct(res->configData);
        SkrDelete(res);
    };
    return true;
}

void SConfigFactory::Serialize(const skr_config_resource_t& config, skr_binary_writer_t& archive)
{
    namespace bin = skr::binary;
    bin::Archive(&archive, config.configType);
    SConfigFactory::SerializeConfig(config.configType, config.configData, archive);
}

void SConfigFactory::DeserializeConfig(const skr_type_id_t& id, void* address, skr_binary_reader_t& archive)
{
    auto registry = GetConfigRegistry();
    auto typeInfo = registry->FindConfigType(id);
    SKR_ASSERT(typeInfo);
    typeInfo->Deserialize(address, archive);
}

void SConfigFactory::SerializeConfig(const skr_type_id_t& id, void* address, skr_binary_writer_t& archive)
{
    auto registry = GetConfigRegistry();
    auto typeInfo = registry->FindConfigType(id);
    SKR_ASSERT(typeInfo);
    typeInfo->Serialize(address, archive);
}
} // namespace resource
} // namespace skr
