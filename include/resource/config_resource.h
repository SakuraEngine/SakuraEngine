#pragma once
#include "EASTL/vector.h"
#include "resource/resource_factory.h"
#include "type/type_registry.h"

typedef struct reflect attr(
    "guid" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74",
    "custom_serialize" : true
)
skr_config_resource_t
{
    skr_type_id_t configType;
    void* configData;
}
skr_config_resource_t;

#if defined(__cplusplus)
// TODO: codegen
constexpr skr_type_id_t skr_get_type_id_skr_config_resource_t()
{
    skr_type_id_t id{ 0x8F2DE9A2, 0xFE05, 0x4EB7, { 0xA0, 0x7F, 0xA9, 0x73, 0xE3, 0xE9, 0x2B, 0x74 } };
    return id;
};

namespace skr
{
namespace resource
{
struct SConfigFactory : public SResourceFactory {
    skr_type_id_t GetResourceType() override { return skr_get_type_id_skr_config_resource_t(); }
    bool Deserialize(skr_resource_record_t* record, SBinaryDeserializer& archive) override;
    static void Serialize(const skr_config_resource_t& config, SBinarySerializer& archive);
    static void DeserializeConfig(const skr_type_id_t& id, void* address, SBinaryDeserializer& archive);
    static void SerializeConfig(const skr_type_id_t& id, void* address, SBinarySerializer& archive);
    static void RegisterConfigType(const skr_type_id_t& id, void (*Serialize)(void* address, SBinaryArchive& archive));
};
} // namespace resource
} // namespace skr
#endif