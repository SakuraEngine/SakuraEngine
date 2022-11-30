#pragma once
#include "shader_resource.hpp"
#ifndef __meta__
#include "SkrRenderer/resources/shader_meta_resource.generated.h"
#endif

sreflect_enum_class("guid": "c289eaaf-ace9-4a86-8072-b173377f7d19")
sattr("serialize" : ["json", "bin"], "rtti" : true)
ESkrShaderOptionType : uint32_t
{
    LEVEL = 0,  // [ "SM_5_0", "SM_6_3", "SM_6_6" ]
    VALUE = 1, // "ATOMIC_BOOL": ["on", "off"]
    SELECT = 2, // [ "BRAND_NVIDIA", "BRAND_INTEL", "BRAND_AMD" ]
    COUNT
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

    sattr("no-rtti" : true) SKR_RENDERER_API
    static skr_stable_shader_hash_t calculate_stable_hash(skr::span<skr_shader_option_instance_t> ordered_options);
};

sreflect_struct("guid" : "f497b62d-e63e-4ec3-b923-2a01a90f9966")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_shader_option_t
{
    ESkrShaderOptionType type;
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

    eastl::vector<skr_shader_option_t> options;
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
}
}