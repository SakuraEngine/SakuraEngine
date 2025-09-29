#pragma once
#include "SkrRuntime/resource/resource_factory.hpp"
#include "SkrRenderer/fwd_types.h"
#include "SkrContainers/string.hpp"
#include "SkrContainers/span.hpp"
#include "SkrRenderer/resources/shader_meta_resource.generated.h" // IWYU pragma: export

namespace skr
{

enum class [[sattr(guid = "c289eaaf-ace9-4a86-8072-b173377f7d19" serde = @enable
)]] EShaderOptionType : uint32_t
{
    LEVEL = 0,  // [ "SM_5_0", "SM_6_3", "SM_6_6" ]
    VALUE = 1,  // "ATOMIC_BOOL": ["on", "off"]
    SELECT = 2, // [ "BRAND_NVIDIA", "BRAND_INTEL", "BRAND_AMD" ]
    COUNT
};

struct [[sattr(guid = "00d4c2b3-50e7-499b-9cf3-fb6b2ba70e79" serde = @enable)]]
ShaderOptionInstance
{
    skr::String key;
    // if value.empty() then it's automatically set to option.value_selections[0] as the default value
    // if value == "on" then it will behave like "-D${key}", ["SOME_MACRO", ""] -> -DSOME_MACRO
    // if value == "off" then it will keep undefined during compile time
    skr::String value;

    SKR_RENDERER_API
    static StableShaderHash calculate_stable_hash(skr::Span<ShaderOptionInstance> ordered_options);
};

struct [[sattr(guid = "f497b62d-e63e-4ec3-b923-2a01a90f9966" serde = @enable)]]
ShaderOptionTemplate
{
    EShaderOptionType type;
    skr::String key;
    skr::Vector<skr::String> value_selections; // { "on", "off" } or { "1", "2", "3" }
    // TODO: target platforms filter
};

struct [[sattr(
    guid = "fc9b4a8e-06c7-41e2-a159-f4cf6930ccfc" serde = @enable)]]
ShaderOptionsResource
{
    using shader_options_handle_t = skr::AsyncResource<ShaderOptionsResource>;

    SKR_RENDERER_API
    static bool flatten_options(skr::Vector<ShaderOptionTemplate>& dst, skr::Span<ShaderOptionsResource*> srcs) SKR_NOEXCEPT;

    skr::Vector<ShaderOptionTemplate> options;
};

struct SKR_RENDERER_API ShaderOptionsFactory : public ResourceFactory
{
    virtual ~ShaderOptionsFactory() = default;

    struct Root
    {
        int __nothing__;
    };

    float AsyncSerdeLoadFactor() override { return 0.1f; }
    [[nodiscard]] static ShaderOptionsFactory* Create(const Root& root);
    static void Destroy(ShaderOptionsFactory* factory);
};
} // namespace skr