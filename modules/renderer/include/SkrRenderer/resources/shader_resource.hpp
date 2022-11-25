#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/fwd_types.h"
#include "platform/filesystem.hpp"
#include "cgpu/api.h"
#include "resource/resource_factory.h"
#include "utils/io.h"
#include <containers/span.hpp>
#include <containers/string.hpp>
#include <containers/hashmap.hpp>
#include <EASTL/vector.h>
#ifndef __meta__
#include "SkrRenderer/resources/shader_resource.generated.h"
#endif

sreflect_struct("guid" : "5a54720c-34b2-444c-8e3a-5977c94136c3")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_stable_shader_hash_t 
{
    inline skr_stable_shader_hash_t() = default;

    uint64_t value = 0;
    
    sattr("no-rtti" : true)
    inline bool operator==(const skr_stable_shader_hash_t& other) const { return value == other.value; }

    struct hasher
    {
        inline size_t operator()(const skr_stable_shader_hash_t& hash) const { return hash.value; }
    };

    SKR_RENDERER_API skr_stable_shader_hash_t(const char* str) SKR_NOEXCEPT;
    SKR_RENDERER_API operator skr::string() const SKR_NOEXCEPT;
};

sreflect_struct("guid" : "0291f512-747e-4b64-ba5c-5fdc412220a3")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_platform_shader_hash_t 
{
    uint32_t flags;
    uint32_t encoded_digits[4];
};

sreflect_struct("guid" : "b0b69898-166f-49de-a675-7b04405b98b1")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_platform_shader_identifier_t 
{
    skr::TEnumAsByte<ECGPUShaderBytecodeType> bytecode_type;
    skr_platform_shader_hash_t hash;
    skr::string entry;
};

sreflect_struct("guid" : "6c07aa34-249f-45b8-8080-dd2462ad5312")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_platform_shader_resource_t
{
    eastl::vector<skr_platform_shader_identifier_t> identifiers;
    skr::TEnumAsByte<ECGPUShaderStage> shader_stage;

    sattr("transient": true, "no-rtti" : true)
    CGPUShaderLibraryId shader;
    sattr("transient": true, "no-rtti" : true)
    uint32_t active_slot;
};

sreflect_struct("guid" : "00d4c2b3-50e7-499b-9cf3-fb6b2ba70e79")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_shader_option_instance_t
{
    eastl::string key;
    // if value.empty() then it's automatically set to option.value_selections[0] as the default value
    // if value == "on" then it will behave like "-D${key}", ["SOME_MACRO", ""] -> -DSOME_MACRO
    // if value == "off" then it will keep undefined during compile time
    eastl::string value;
};

sreflect_struct("guid" : "f497b62d-e63e-4ec3-b923-2a01a90f9966")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_shader_option_t
{
    eastl::string key;
    eastl::vector<eastl::string> value_selections; // { "on", "off" } or { "1", "2", "3" }
    // TODO: target platforms filter
};

sreflect_struct("guid" : "fc9b4a8e-06c7-41e2-a159-f4cf6930ccfc")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_shader_options_resource_t
{
    using shader_options_handle_t = skr::resource::TResourceHandle<skr_shader_options_resource_t>;

    sattr("no-rtti" : true) SKR_RENDERER_API
    static bool flatten_options(eastl::vector<skr_shader_option_t>& dst, skr::span<skr_shader_options_resource_t*> srcs) SKR_NOEXCEPT;

    // TODO: replace this with set when rtti & serde is ready
    eastl::vector<skr_shader_option_t> options;
};

sreflect_struct("guid" : "1c7d845a-fde8-4487-b1c9-e9c48d6a9867")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_platform_shader_collection_resource_t
{
    using stable_hash = skr_stable_shader_hash_t;
    using stable_hasher = skr_stable_shader_hash_t::hasher;

    skr_guid_t root_guid;
    // hash=0 -> root_variant;
    sattr("no-rtti" : true)
    skr::flat_hash_map<stable_hash, skr_platform_shader_resource_t, stable_hasher> variants;
};

namespace skr sreflect
{
namespace resource sreflect
{
struct SKR_RENDERER_API SShaderOptionsFactory : public SResourceFactory {
    virtual ~SShaderOptionsFactory() = default;

    struct Root {
        int __nothing__;
    };

    float AsyncSerdeLoadFactor() override { return 0.1f; }
    [[nodiscard]] static SShaderOptionsFactory* Create(const Root& root);
    static void Destroy(SShaderOptionsFactory* factory); 
};

struct SKR_RENDERER_API SShaderResourceFactory : public SResourceFactory {
    virtual ~SShaderResourceFactory() = default;

    struct Root {
        skr_vfs_t* bytecode_vfs = nullptr;
        skr_io_ram_service_t* ram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
        skr_threaded_service_t* aux_service = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 1.f; }
    [[nodiscard]] static SShaderResourceFactory* Create(const Root& root);
    static void Destroy(SShaderResourceFactory* factory); 
};
} // namespace resource
} // namespace skr