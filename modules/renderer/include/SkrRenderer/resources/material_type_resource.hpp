#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"

#ifndef __meta__
#include "SkrRenderer/resources/material_type_resource.generated.h"
#endif

namespace skr sreflect
{
namespace renderer sreflect
{
using MaterialPropertyName = skr::string;

sreflect_enum_class("guid" : "4003703a-dde4-4f11-93a6-6c460bac6357")
sattr("rtti": true, "serialize": ["json", "bin"])
EMaterialPropertyType : uint32_t
{
    BOOL,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    DOUBLE,
    TEXTURE,
    BUFFER,
    SAMPLER,
    STATIC_SAMPLER,
    COUNT
};

sreflect_enum("guid" : "575331c4-785f-4a4d-b320-4490bb7a6180")
sattr("rtti": true, "serialize" : ["json", "bin"])
EMaterialBlendMode : uint32_t
{
    Opaque,
    Blend,
    Mask,
    Count
};

sreflect_struct("guid": "6cdbf15e-67c1-45c1-a4e9-417c81299dae")
sattr("rtti": true, "serialize": ["json", "bin"])
MaterialProperty
{
    using resource_handle = skr_resource_handle_t;

    MaterialPropertyName name;
    skr::string display_name;
    EMaterialPropertyType prop_type;
    skr::string description;

    double default_value = 0.0;
    double min_value = 0.0;
    double max_value = DBL_MAX;

    skr_float4_t default_vec;
    skr_float4_t min_vec = { 0.0f, 0.0f, 0.0f, 0.0f };
    skr_float4_t max_vec = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

    resource_handle default_resource = {};
};

// material value setter mainly used for devlopment-time material editing
// at runtime we use skr_material_value_$(type)_t
sreflect_struct("guid": "46de11b4-6beb-4ab9-b9f8-f5c07ceeb8a5")
sattr("rtti": true, "serialize": ["json", "bin"])
MaterialValue
{
    using resource_handle = skr_resource_handle_t;

    EMaterialPropertyType prop_type;
    MaterialPropertyName slot_name;

    double value = 0.0;
    skr_float4_t vec = { 0.0f, 0.0f, 0.0f, 0.0f };
    resource_handle resource;
};

sreflect_struct("guid" : "ed2e3476-90a3-4f2f-ac97-808f63d1eb11")
sattr("rtti": true, "serialize" : ["json", "bin"])
MaterialPass
{
    skr::string pass;
    skr::vector<skr_shader_collection_handle_t> shader_resources;
    skr::vector<EMaterialBlendMode> blend_modes;
    bool two_sided = false;
};

sreflect_struct("guid" : "83264b35-3fde-4fff-8ee1-89abce2e445b")
sattr("rtti": true, "serialize" : ["json", "bin"])
MaterialTypeResource
{
    uint32_t version;
    
    skr::vector<MaterialPass> passes;
    skr::vector<MaterialValue> default_values;
    skr::vector<skr_shader_option_instance_t> switch_defaults;
    skr::vector<skr_shader_option_instance_t> option_defaults;
    skr_vertex_layout_id vertex_type;
};

struct SKR_RENDERER_API SMaterialTypeFactory : public resource::SResourceFactory {
    virtual ~SMaterialTypeFactory() = default;

    struct Root {
        SRenderDeviceId render_device = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 1.f; }
    [[nodiscard]] static SMaterialTypeFactory* Create(const Root& root);
    static void Destroy(SMaterialTypeFactory* factory); 
};
} // namespace resource
} // namespace skr