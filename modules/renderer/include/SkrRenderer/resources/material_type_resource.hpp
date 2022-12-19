#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"

#ifndef __meta__
#include "SkrRenderer/resources/material_type_resource.generated.h"
#endif

sreflect_enum_class("guid" : "4003703a-dde4-4f11-93a6-6c460bac6357")
sattr("rtti": true, "serialize": ["json", "bin"])
ESkrMaterialPropertyType : uint32_t
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
    COUNT
};

using skr_material_property_name_t = skr::string;

sreflect_struct("guid": "6cdbf15e-67c1-45c1-a4e9-417c81299dae")
sattr("rtti": true, "serialize": ["json", "bin"])
skr_material_property_t
{
    using resource_handle = skr_resource_handle_t;

    skr_material_property_name_t name;
    skr::string display_name;
    ESkrMaterialPropertyType prop_type;
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
skr_material_value_t
{
    using resource_handle = skr_resource_handle_t;

    ESkrMaterialPropertyType prop_type;
    skr_material_property_name_t slot_name;

    double value = 0.0;
    skr_float4_t vec = { 0.0f, 0.0f, 0.0f, 0.0f };
    resource_handle resource;
};

sreflect_struct("guid" : "83264b35-3fde-4fff-8ee1-89abce2e445b")
sattr("rtti": true, "serialize" : ["json", "bin"])
skr_material_type_resource_t
{
    uint32_t version;
    
    skr::vector<skr_shader_collection_handle_t> shader_resources;
    skr::vector<skr_material_value_t> default_values;
    skr::vector<skr_shader_option_instance_t> switch_defaults;
    skr::vector<skr_shader_option_instance_t> option_defaults;
    skr_vertex_layout_id vertex_type;
};

namespace skr sreflect
{
namespace resource sreflect
{
struct SKR_RENDERER_API SMaterialTypeFactory : public SResourceFactory {
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