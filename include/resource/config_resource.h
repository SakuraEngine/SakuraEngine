#pragma once
#include "EASTL/vector.h"
#include "utils/hashmap.hpp"
#include "resource/resource_factory.h"
#include "type/type_registry.h"

typedef struct sreflect sattr(
    "guid" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74"
)
skr_config_resource_t
{
    typedef struct skr_config_resource_t RawData;
    skr_type_id_t configType;
    void* configData;
}
skr_config_resource_t;

#if defined(__cplusplus)
inline static constexpr skr_guid_t get_type_id_skr_config_resource_t()
{ return {0x8F2DE9A2, 0xFE05, 0x4EB7, {0xA0, 0x7F, 0xA9, 0x73, 0xE3, 0xE9, 0x2B, 0x74}}; }

    #include "simdjson.h"
namespace skr
{
namespace resource
{
struct SConfigTypeInfo {
    void (*Serialize)(void* address, SBinaryArchive& archive);
};
struct SConfigRegistry {
    skr::flat_hash_map<skr_guid_t, SConfigTypeInfo, skr::guid::hash> typeInfos;
};
RUNTIME_API SConfigRegistry* GetConfigRegistry();

struct RUNTIME_API SConfigFactory : public SResourceFactory {
    skr_type_id_t GetResourceType() override;
    static skr_config_resource_t* NewConfig(skr_type_id_t& id);
    bool AsyncIO() override { return false; }
    ESkrLoadStatus Load(skr_resource_record_t* record) override;
    bool Deserialize(skr_resource_record_t* record, SBinaryDeserializer& archive);
    static void Serialize(const skr_config_resource_t& config, SBinarySerializer& archive);
    static void DeserializeConfig(const skr_type_id_t& id, void* address, SBinaryDeserializer& archive);
    static void SerializeConfig(const skr_type_id_t& id, void* address, SBinarySerializer& archive);
};
} // namespace resource
} // namespace skr
#endif