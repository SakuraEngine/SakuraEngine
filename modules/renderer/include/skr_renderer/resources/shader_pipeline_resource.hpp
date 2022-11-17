#pragma once
#include "shader_resource.hpp"

sreflect_struct("guid" : "42b32962-e049-4beb-9209-9673502c901a")
sattr("serialize" : "bin")
skr_shader_pipeline_resource_t
{
    eastl::vector<skr_platform_shader_resource_t> shader_blobs;
};
typedef struct skr_shader_pipeline_resource_t skr_shader_pipeline_resource_t;

namespace skr sreflect
{
namespace resource sreflect
{
struct SKR_RENDERER_API SShaderPipelineFactory : public SResourceFactory {
    virtual ~SShaderPipelineFactory() = default;

    struct Root {
        skr_vfs_t* bytecode_vfs = nullptr;
        skr_io_ram_service_t* ram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
    };

    [[nodiscard]] static SShaderPipelineFactory* Create(const Root& root);
    static void Destroy(SShaderPipelineFactory* factory); 
};
} // namespace resource
} // namespace skr