#pragma once
#include "SkrBase/types.h"
#include "SkrRuntime/config.h"
#include "SkrRuntime/resource/resource_factory.hpp"
#include <SkrBase/type_info.hpp>

typedef struct skr_config_resource_t skr_config_resource_t;

#if defined(__cplusplus)
struct [[sattr(guid = "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74"
)]] SKR_RUNTIME_API skr_config_resource_t
{
    skr::GUID configType;
    void* configData = nullptr;
    void SetType(skr::GUID type);
    ~skr_config_resource_t();
};

SKR_TYPE_INFO(skr_config_resource_t, "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74")

namespace skr
{
template <>
struct SKR_RUNTIME_API Serialize<skr_config_resource_t>
{
    static void read(ArchiveRead& r, skr_config_resource_t& v);
    static void write(ArchiveWrite& w, const skr_config_resource_t& v);
};
} // namespace skr

namespace skr
{
struct SKR_RUNTIME_API ConfigFactory : public ResourceFactory
{
    skr::GUID GetResourceType() override;

    bool AsyncIO() override { return false; }
};

} // namespace skr
#endif