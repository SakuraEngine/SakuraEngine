#pragma once
#include "SkrRenderer/module.configure.h"
#include "skr_renderer/fwd_types.h"
#include "cgpu/flags.h"
#include <EASTL/vector.h>
#include <EASTL/string.h>
#ifndef __meta__
    #include "SkrRenderer/resources/shader_resource.generated.h"
#endif

sreflect_struct("guid" : "0291f512-747e-4b64-ba5c-5fdc412220a3")
sattr("serialize" : ["json", "bin"])
skr_shader_hash_t 
{
    uint32_t flags;
    skr_guid_t encoded_digits;
};
typedef struct skr_shader_hash_t skr_shader_hash_t;

sreflect_struct("guid" : "b0b69898-166f-49de-a675-7b04405b98b1")
sattr("serialize" : ["json", "bin"])
skr_platform_shader_identifier_t 
{
    uint32_t bytecode_type; // ECGPUShaderBytecodeType
    skr_shader_hash_t hash;
    eastl::string entry;
};
typedef struct skr_platform_shader_identifier_t skr_platform_shader_identifier_t;

sreflect_struct("guid" : "1c7d845a-fde8-4487-b1c9-e9c48d6a9867")
sattr("serialize" : ["json", "bin"])
skr_shader_pipeline_resource_t
{
    eastl::vector<skr_platform_shader_identifier_t> shader_blobs;
};
typedef struct skr_shader_pipeline_resource_t skr_shader_pipeline_resource_t;


#ifdef __cplusplus
#include "platform/filesystem.hpp"
#include "resource/resource_factory.h"
#include "utils/io.h"

namespace skr { namespace io { class VRAMService; } }
namespace skr sreflect
{
namespace resource sreflect
{
struct SKR_RENDERER_API SShaderPipelineFactory : public SResourceFactory {
    virtual ~SShaderPipelineFactory() = default;

    struct Root {
        skr_vfs_t* texture_vfs = nullptr;
        skr::filesystem::path dstorage_root;
        skr_io_ram_service_t* ram_service = nullptr;
        skr::io::VRAMService* vram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
    };

    [[nodiscard]] static SShaderPipelineFactory* Create(const Root& root);
    static void Destroy(SShaderPipelineFactory* factory); 
};
} // namespace resource
} // namespace skr
#endif