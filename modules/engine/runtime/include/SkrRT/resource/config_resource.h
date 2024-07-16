#pragma once
#include "SkrBase/types.h"
#include "SkrRT/config.h"
#include "SkrRT/resource/resource_factory.h"
#include "SkrRTTR/rttr_traits.hpp"

typedef struct skr_config_resource_t skr_config_resource_t;

#if defined(__cplusplus)
sreflect_struct("guid" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74")
SKR_RUNTIME_API skr_config_resource_t {
    skr_guid_t configType;
    void*      configData = nullptr;
    void       SetType(skr_guid_t type);
    ~skr_config_resource_t();
};

SKR_RTTR_TYPE(skr_config_resource_t, "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74")

namespace skr
{
template <>
struct SKR_RUNTIME_API BinSerde<skr_config_resource_t> {
    static bool read(SBinaryReader* r, skr_config_resource_t& v);
    static bool write(SBinaryWriter* w, const skr_config_resource_t& v);
};
} // namespace skr

namespace skr
{
namespace resource
{
struct SKR_RUNTIME_API SConfigFactory : public SResourceFactory {
    skr_guid_t GetResourceType() override;

    bool AsyncIO() override { return false; }
};
} // namespace resource
} // namespace skr
#endif