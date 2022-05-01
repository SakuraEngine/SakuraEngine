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
namespace skr
{
namespace resource
{
struct SConfigFactory : public SResourceFactory {
    skr_type_id_t GetResourceType() override;
    bool Deserialize(skr_resource_record_t* record, SBinaryArchive& archive) override;
};
} // namespace resource
} // namespace skr
#endif