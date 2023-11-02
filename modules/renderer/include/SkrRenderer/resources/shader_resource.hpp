#pragma once
#include "SkrRenderer/shader_hash.h"
#include "SkrRT/io/ram_io.hpp"
#include "cgpu/api.h"
#include "SkrRT/resource/resource_factory.h"
#include <SkrRT/containers/string.hpp>
#include <SkrRT/containers/hashmap.hpp>

#ifndef __meta__
    #include "SkrRenderer/resources/shader_resource.generated.h" // IWYU pragma: export
#endif

namespace skr sreflect
{
namespace renderer sreflect
{

sreflect_struct("guid" : "6c07aa34-249f-45b8-8080-dd2462ad5312")
sattr("serialize" : ["json", "bin"], "rtti" : true)
MultiShaderResource
{
    using stable_hash_t = skr_stable_shader_hash_t;
    using stable_hasher_t = skr_stable_shader_hash_t::hasher;

    stable_hash_t stable_hash;
    skr::StronglyEnum<ECGPUShaderStage> shader_stage;
    skr::string entry;

    sattr("no-rtti" : true)
    inline skr::vector<skr_platform_shader_identifier_t>& GetRootDynamicVariants() SKR_NOEXCEPT{
        return GetDynamicVariants(kZeroStableShaderHash);
    }
    sattr("no-rtti" : true)
    inline skr::vector<skr_platform_shader_identifier_t>& GetDynamicVariants(stable_hash_t hash) SKR_NOEXCEPT{
        auto found = option_variants.find(hash);
        SKR_ASSERT(found != option_variants.end());
        return found->second;
    }

    sattr("no-rtti" : true)
    skr::flat_hash_map<stable_hash_t, skr::vector<skr_platform_shader_identifier_t>, stable_hasher_t> option_variants;
};

sreflect_struct("guid": "8372f075-b4ce-400d-929f-fb0e57c1c887")
sattr("blob" : true)
ShaderOptionSequence
{
    skr::span<EShaderOptionType> types;
    skr::span<skr::string_view> keys;
    skr::span<skr::span<skr::string_view>> values;

    sattr("no-rtti" : true) SKR_RENDERER_API
    uint32_t find_key_index(skr::string_view key) const SKR_NOEXCEPT;

    sattr("no-rtti" : true) SKR_RENDERER_API
    uint32_t find_value_index(skr::string_view key, skr::string_view value) const SKR_NOEXCEPT;

    sattr("no-rtti" : true) SKR_RENDERER_API
    uint32_t find_value_index(uint32_t key_index, skr::string_view value) const SKR_NOEXCEPT;

    sattr("no-rtti" : true) SKR_RENDERER_API
    static skr_stable_shader_hash_t calculate_stable_hash(const ShaderOptionSequence& seq, skr::span<uint32_t> indices);
};
GENERATED_BLOB_BUILDER(ShaderOptionSequence)

sreflect_struct("guid" : "1c7d845a-fde8-4487-b1c9-e9c48d6a9867")
sattr("serialize" : "bin", "rtti" : true)
ShaderCollectionResource
{
    using stable_hash_t = skr_stable_shader_hash_t;
    using stable_hasher_t = skr_stable_shader_hash_t::hasher;
    SKR_RENDERER_API ~ShaderCollectionResource() SKR_NOEXCEPT;

    sattr("no-rtti" : true)
    inline MultiShaderResource& GetRootStaticVariant() SKR_NOEXCEPT {
        return GetStaticVariant(kZeroStableShaderHash);
    }

    sattr("no-rtti" : true)
    inline MultiShaderResource& GetStaticVariant(const skr_stable_shader_hash_t& hash) SKR_NOEXCEPT {
        auto found = switch_variants.find(hash);
        SKR_ASSERT(found != switch_variants.end());
        return found->second;
    }

    skr_guid_t root_guid;
    // hash=0 -> root_variant;
    spush_attr("no-rtti" : true)
    skr::flat_hash_map<stable_hash_t, MultiShaderResource, stable_hasher_t> switch_variants;

    skr_blob_arena_t switch_arena;
    skr_blob_arena_t option_arena;

    sattr("arena" : "switch_arena")
    skr_shader_option_sequence_t switch_sequence;
    sattr("arena" : "option_arena")
    skr_shader_option_sequence_t option_sequence;
};

sreflect_struct("guid" : "a633ea13-53d8-4202-b6f1-ec882ac409ec")
sattr("serialize" : ["json", "bin"], "rtti" : true)
ShaderCollectionJSON
{
    using stable_hash_t = skr_stable_shader_hash_t;
    using stable_hasher_t = skr_stable_shader_hash_t::hasher;

    sattr("no-rtti" : true)
    inline MultiShaderResource& GetRootStaticVariant() SKR_NOEXCEPT {
        return GetStaticVariant(kZeroStableShaderHash);
    }

    sattr("no-rtti" : true)
    inline MultiShaderResource& GetStaticVariant(const skr_stable_shader_hash_t& hash) SKR_NOEXCEPT {
        auto found = switch_variants.find(hash);
        SKR_ASSERT(found != switch_variants.end());
        return found->second;
    }

    skr_guid_t root_guid;
    // hash=0 -> root_variant;
    sattr("no-rtti" : true)
    skr::flat_hash_map<stable_hash_t, MultiShaderResource, stable_hasher_t> switch_variants;
    
    skr::vector<skr::string> switch_key_sequence;
    skr::vector<EShaderOptionType> switch_type_sequence;
    skr::vector<skr::vector<skr::string>> switch_values_sequence;
    skr::vector<skr::string> option_key_sequence;
    skr::vector<EShaderOptionType> option_type_sequence;
    skr::vector<skr::vector<skr::string>> option_values_sequence;
};

struct SKR_RENDERER_API SShaderResourceFactory : public resource::SResourceFactory {
    virtual ~SShaderResourceFactory() = default;

    struct Root {
        skr_shader_map_t* shadermap = nullptr;
        SRenderDeviceId render_device = nullptr;
        bool dont_create_shader = false;
    };

    float AsyncSerdeLoadFactor() override { return 1.f; }
    [[nodiscard]] static SShaderResourceFactory* Create(const Root& root);
    static void Destroy(SShaderResourceFactory* factory); 

    static ECGPUShaderBytecodeType GetRuntimeBytecodeType(ECGPUBackend backend);
};
} // namespace resource
} // namespace skr
namespace skr::binary { template <> struct BlobBuilderType<skr_stable_shader_hash_t> { using type = skr_stable_shader_hash_t; }; }