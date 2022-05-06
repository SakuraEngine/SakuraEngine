#pragma once
#include "EASTL/vector.h"
#include "phmap.h"
#include "resource/resource_factory.h"
#include "type/type_registry.h"

typedef struct reflect attr(
    "guid" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74"
)
skr_config_resource_t
{
    skr_type_id_t configType;
    void* configData;
}
skr_config_resource_t;

#if defined(__cplusplus)
    #include "simdjson.h"
    #ifndef __meta__
        #include "SkrRT/typeid.generated.hpp"
    #endif
namespace skr
{
namespace resource
{
struct SConfigTypeInfo {
    void (*Serialize)(void* address, SBinaryArchive& archive);
};
struct SConfigRegistry {
    phmap::flat_hash_map<skr_guid_t, SConfigTypeInfo, skr::guid::hash> typeInfos;
};
RUNTIME_API SConfigRegistry* GetConfigRegistry();

struct SConfigFactory : public SResourceFactory {
    skr_type_id_t GetResourceType() override;
    bool Deserialize(skr_resource_record_t* record, SBinaryDeserializer& archive) override;
    static void Serialize(const skr_config_resource_t& config, SBinarySerializer& archive);
    static void DeserializeConfig(const skr_type_id_t& id, void* address, SBinaryDeserializer& archive);
    static void SerializeConfig(const skr_type_id_t& id, void* address, SBinarySerializer& archive);
};
} // namespace resource
} // namespace skr
#endif