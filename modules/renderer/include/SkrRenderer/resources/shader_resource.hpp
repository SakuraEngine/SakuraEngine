#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/fwd_types.h"
#include "platform/filesystem.hpp"
#include "cgpu/api.h"
#include "resource/resource_factory.h"
#include "utils/io.h"
#include <containers/string.hpp>
#include <containers/hashmap.hpp>
#include <EASTL/vector.h>
#ifndef __meta__
#include "SkrRenderer/resources/shader_resource.generated.h"
#endif

sreflect_struct("guid" : "5a54720c-34b2-444c-8e3a-5977c94136c3")
sattr("serialize" : ["json", "bin"])
sattr("rtti" : true)
skr_stable_shader_hash_t 
{
    uint64_t value = 0;
    
    sattr("no-rtti" : true)
    inline bool operator==(const skr_stable_shader_hash_t& other) const { return value == other.value; }

    struct hasher
    {
        inline size_t operator()(const skr_stable_shader_hash_t& hash) const { return hash.value; }
    };
};
typedef struct skr_stable_shader_hash_t skr_stable_shader_hash_t;

sreflect_struct("guid" : "0291f512-747e-4b64-ba5c-5fdc412220a3")
sattr("serialize" : ["json", "bin"])
sattr("rtti" : true)
skr_platform_shader_hash_t 
{
    uint32_t flags;
    uint32_t encoded_digits[4];
};
typedef struct skr_platform_shader_hash_t skr_platform_shader_hash_t;

sreflect_struct("guid" : "b0b69898-166f-49de-a675-7b04405b98b1")
sattr("serialize" : ["json", "bin"])
sattr("rtti" : true)
skr_platform_shader_identifier_t 
{
    skr::TEnumAsByte<ECGPUShaderBytecodeType> bytecode_type;
    skr_platform_shader_hash_t hash;
    skr::string entry;
};
typedef struct skr_platform_shader_identifier_t skr_platform_shader_identifier_t;

sreflect_struct("guid" : "6c07aa34-249f-45b8-8080-dd2462ad5312")
sattr("serialize" : "bin")
sattr("rtti" : true)
skr_platform_shader_resource_t
{
    eastl::vector<skr_platform_shader_identifier_t> identifiers;
    sattr("no-rtti" : true)
    ECGPUShaderStage shader_stage;

    sattr("transient": true, "no-rtti" : true)
    CGPUShaderLibraryId shader;
    sattr("transient": true, "no-rtti" : true)
    uint32_t active_slot;
};
typedef struct skr_platform_shader_resource_t skr_platform_shader_resource_t;

sreflect_struct("guid" : "1c7d845a-fde8-4487-b1c9-e9c48d6a9867")
sattr("serialize" : "bin")
sattr("rtti" : true)
skr_platform_shader_collection_resource_t
{
    using stable_hash = skr_stable_shader_hash_t;
    using stable_hasher = skr_stable_shader_hash_t::hasher;

    skr_guid_t asset_guid;
    // hash=0 -> root_variant;
    sattr("no-rtti" : true)
    skr::flat_hash_map<stable_hash, skr_platform_shader_resource_t, stable_hasher> variants;
};
typedef struct skr_platform_shader_collection_resource_t skr_platform_shader_collection_resource_t;

namespace skr sreflect
{
namespace resource sreflect
{
struct SKR_RENDERER_API SShaderResourceFactory : public SResourceFactory {
    virtual ~SShaderResourceFactory() = default;

    struct Root {
        skr_vfs_t* bytecode_vfs = nullptr;
        skr_io_ram_service_t* ram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
        skr_threaded_service_t* aux_service = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 0.f; }
    [[nodiscard]] static SShaderResourceFactory* Create(const Root& root);
    static void Destroy(SShaderResourceFactory* factory); 
};
} // namespace resource
} // namespace skr