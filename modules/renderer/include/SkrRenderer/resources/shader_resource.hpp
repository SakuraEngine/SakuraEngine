#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/fwd_types.h"
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
    inline constexpr skr_stable_shader_hash_t() = default;

    uint32_t valuea = 0;
    uint32_t valueb = 0;
    uint32_t valuec = 0;
    uint32_t valued = 0;
    
    sattr("no-rtti" : true)
    inline bool operator==(const skr_stable_shader_hash_t& other) const
    {
        return valuea == other.valuea && valueb == other.valueb && valuec == other.valuec && valued == other.valued;
    }

    struct hasher
    {
        SKR_RENDERER_API size_t operator()(const skr_stable_shader_hash_t& hash) const;
    };

    sattr("no-rtti" : true)
    SKR_RENDERER_API static skr_stable_shader_hash_t from_string(const char* str) SKR_NOEXCEPT;
    SKR_RENDERER_API skr_stable_shader_hash_t(uint32_t a, uint32_t b, uint32_t c, uint32_t d) SKR_NOEXCEPT;
    SKR_RENDERER_API explicit operator skr::string() const SKR_NOEXCEPT;
};
static constexpr skr_stable_shader_hash_t kZeroStableShaderHash = skr_stable_shader_hash_t();

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
    skr::TEnumAsByte<ECGPUShaderStage> shader_stage;
    skr_platform_shader_hash_t hash;
    skr::string entry;
};

sreflect_struct("guid" : "6c07aa34-249f-45b8-8080-dd2462ad5312")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_platform_shader_resource_t
{
    using stable_hash_t = skr_stable_shader_hash_t;
    using stable_hasher_t = skr_stable_shader_hash_t::hasher;

    stable_hash_t stable_hash;
    skr::TEnumAsByte<ECGPUShaderStage> shader_stage;

    sattr("no-rtti" : true)
    inline eastl::vector<skr_platform_shader_identifier_t>& GetRootDynamicVariants() SKR_NOEXCEPT{
        auto found = option_variants.find(kZeroStableShaderHash);
        SKR_ASSERT(found != option_variants.end());
        return found->second;
    }
    sattr("no-rtti" : true)
    inline eastl::vector<skr_platform_shader_identifier_t>& GetDynamicVariants(stable_hash_t hash) SKR_NOEXCEPT{
        auto found = option_variants.find(hash);
        SKR_ASSERT(found != option_variants.end());
        return found->second;
    }

    sattr("no-rtti" : true)
    skr::flat_hash_map<stable_hash_t, eastl::vector<skr_platform_shader_identifier_t>, stable_hasher_t> option_variants;

    sattr("transient": true, "no-rtti" : true)
    CGPUShaderLibraryId shader = nullptr;
    sattr("transient": true, "no-rtti" : true)
    uint32_t active_slot = 0;
};

sreflect_struct("guid" : "1c7d845a-fde8-4487-b1c9-e9c48d6a9867")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_platform_shader_collection_resource_t
{
    using stable_hash_t = skr_stable_shader_hash_t;
    using stable_hasher_t = skr_stable_shader_hash_t::hasher;

    sattr("no-rtti" : true)
    inline skr_platform_shader_resource_t& GetRootStaticVariant() SKR_NOEXCEPT {
        auto found = switch_variants.find(kZeroStableShaderHash);
        SKR_ASSERT(found != switch_variants.end());
        return found->second;
    }

    skr_guid_t root_guid;
    // hash=0 -> root_variant;
    sattr("no-rtti" : true)
    skr::flat_hash_map<stable_hash_t, skr_platform_shader_resource_t, stable_hasher_t> switch_variants;
};

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

    float AsyncSerdeLoadFactor() override { return 1.f; }
    [[nodiscard]] static SShaderResourceFactory* Create(const Root& root);
    static void Destroy(SShaderResourceFactory* factory); 
};
} // namespace resource
} // namespace skr