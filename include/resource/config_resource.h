#pragma once
#include "resource/resource_factory.h"
#include "type/type_registry.h"

extern "C" struct reflect attr("guid" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74")
skr_config_resource_t
{
    skr_type_id_t configType;
    void* configData;
};
#if defined(__cplusplus)
// TODO: codegen
constexpr skr_type_id_t get_type_id_skr_config_resource_t()
{
    skr_type_id_t id{ 0x8F2DE9A2, 0xFE05, 0x4EB7, { 0xA0, 0x7F, 0xA9, 0x73, 0xE3, 0xE9, 0x2B, 0x74 } };
    return id;
};

namespace skr
{
namespace resource
{
void DeserializeConfig(const skr_type_id_t& id, void* address, SBinaryArchive& archive);
struct SConfigFactory : public SResourceFactory {
    skr_type_id_t GetResourceType() override { return get_type_id_skr_config_resource_t(); }
    bool Deserialize(skr_resource_record_t* record, SBinaryArchive& archive) override;
};
} // namespace resource
} // namespace skr
#endif