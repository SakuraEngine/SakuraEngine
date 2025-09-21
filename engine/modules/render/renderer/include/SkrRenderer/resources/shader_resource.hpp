#pragma once
#include "SkrGraphics/api.h"
#include "SkrRuntime/resource/resource_factory.h"
#include "SkrRenderer/graphics/shader_hash.hpp"
#include <SkrContainers/string.hpp>
#include <SkrContainers/hashmap.hpp>

#include "SkrRenderer/resources/shader_resource.generated.h" // IWYU pragma: export

namespace skr
{
struct [[sattr(
    guid = "6c07aa34-249f-45b8-8080-dd2462ad5312"
    serde = @enable
)]] MultiShaderResource
{
    StableShaderHash stable_hash;
    skr::EnumAsValue<ECGPUShaderStage> shader_stage;
    skr::String entry;

    inline skr::Vector<PlatformShaderIdentifier>& GetRootDynamicVariants() SKR_NOEXCEPT
    {
        return GetDynamicVariants(kZeroStableShaderHash);
    }

    inline skr::Vector<PlatformShaderIdentifier>& GetDynamicVariants(StableShaderHash hash) SKR_NOEXCEPT
    {
        auto found = option_variants.find(hash);
        SKR_ASSERT(found != option_variants.end());
        return found->second;
    }

    skr::FlatHashMap<StableShaderHash, skr::Vector<PlatformShaderIdentifier>, StableShaderHash::hasher> option_variants;
};

struct [[sattr(
    guid = "8372f075-b4ce-400d-929f-fb0e57c1c887"
    serde = @enable
)]] ShaderOptionSequence
{
    skr::SerializeConstVector<EShaderOptionType> types;
    skr::SerializeConstVector<skr::SerializeConstString> keys;
    skr::SerializeConstVector<skr::SerializeConstVector<skr::SerializeConstString>> values;

    SKR_RENDERER_API
    uint32_t find_key_index(skr::StringView key) const SKR_NOEXCEPT;

    SKR_RENDERER_API
    uint32_t find_value_index(skr::StringView key, skr::StringView value) const SKR_NOEXCEPT;

    SKR_RENDERER_API
    uint32_t find_value_index(uint32_t key_index, skr::StringView value) const SKR_NOEXCEPT;

    SKR_RENDERER_API
    static StableShaderHash calculate_stable_hash(const ShaderOptionSequence& seq, skr::Span<uint32_t> indices);
};

struct [[sattr(
    guid = "1c7d845a-fde8-4487-b1c9-e9c48d6a9867" 
    serde = @enable
)]] ShaderCollectionResource
{
    using stable_hash_t = StableShaderHash;
    using stable_hasher_t = StableShaderHash::hasher;
    SKR_RENDERER_API ~ShaderCollectionResource() SKR_NOEXCEPT;

    inline MultiShaderResource& GetRootStaticVariant() SKR_NOEXCEPT
    {
        return GetStaticVariant(kZeroStableShaderHash);
    }

    inline MultiShaderResource& GetStaticVariant(const StableShaderHash& hash) SKR_NOEXCEPT
    {
        auto found = switch_variants.find(hash);
        SKR_ASSERT(found != switch_variants.end());
        return found->second;
    }

    GUID root_guid;
    // hash=0 -> root_variant;
    skr::FlatHashMap<stable_hash_t, MultiShaderResource, stable_hasher_t> switch_variants;

    ShaderOptionSequence switch_sequence;
    ShaderOptionSequence option_sequence;
};

struct [[sattr(
    guid = "a633ea13-53d8-4202-b6f1-ec882ac409ec" 
    serde = @enable
)]] ShaderCollectionJSON
{
    using stable_hash_t = StableShaderHash;
    using stable_hasher_t = StableShaderHash::hasher;

    inline MultiShaderResource& GetRootStaticVariant() SKR_NOEXCEPT
    {
        return GetStaticVariant(kZeroStableShaderHash);
    }

    inline MultiShaderResource& GetStaticVariant(const StableShaderHash& hash) SKR_NOEXCEPT
    {
        auto found = switch_variants.find(hash);
        SKR_ASSERT(found != switch_variants.end());
        return found->second;
    }

    GUID root_guid;
    // hash=0 -> root_variant;
    skr::FlatHashMap<stable_hash_t, MultiShaderResource, stable_hasher_t> switch_variants;

    skr::Vector<skr::String> switch_key_sequence;
    skr::Vector<EShaderOptionType> switch_type_sequence;
    skr::Vector<skr::Vector<skr::String>> switch_values_sequence;
    skr::Vector<skr::String> option_key_sequence;
    skr::Vector<EShaderOptionType> option_type_sequence;
    skr::Vector<skr::Vector<skr::String>> option_values_sequence;
};

struct SKR_RENDERER_API ShaderResourceFactory : public ResourceFactory
{
    virtual ~ShaderResourceFactory() = default;

    struct Root
    {
        ShaderMap* shadermap = nullptr;
        SRenderDeviceId render_device = nullptr;
        bool dont_create_shader = false;
    };

    float AsyncSerdeLoadFactor() override { return 1.f; }
    [[nodiscard]] static ShaderResourceFactory* Create(const Root& root);
    static void Destroy(ShaderResourceFactory* factory);

    static ECGPUShaderBytecodeType GetRuntimeBytecodeType(ECGPUBackend backend);
};
} // namespace skr