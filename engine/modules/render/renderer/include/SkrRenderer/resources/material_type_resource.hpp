#pragma once
#include "SkrBase/config.h"
#include "SkrRenderer/resources/shader_meta_resource.hpp"

#include "SkrRenderer/resources/material_type_resource.generated.h" // IWYU pragma: export

namespace skr
{
using MaterialPropertyName = skr::String;

enum class [[sattr(
    guid = "4003703a-dde4-4f11-93a6-6c460bac6357";
    serde = @enable;
)]] EMaterialPropertyType : uint32_t
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

enum [[sattr(
    guid = "575331c4-785f-4a4d-b320-4490bb7a6180";
    serde = @enable;
)]] EMaterialBlendMode : uint32_t
{
    Opaque,
    Blend,
    Mask,
    Count
};

struct [[sattr(
    guid = "6cdbf15e-67c1-45c1-a4e9-417c81299dae";
    serde = @enable;
)]] MaterialProperty
{
    EMaterialPropertyType prop_type;
    MaterialPropertyName name;
    String display_name;
    String description;

    double default_value = 0.0;
    double min_value = 0.0;
    double max_value = DBL_MAX;

    float4 default_vec;
    float4 min_vec = { 0.0f, 0.0f, 0.0f, 0.0f };
    float4 max_vec = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

    SResourceHandle default_resource;
};

// material value setter mainly used for devlopment-time material editing
// at runtime we use skr_material_value_$(type)_t
struct [[sattr(
    guid = "46de11b4-6beb-4ab9-b9f8-f5c07ceeb8a5";
    serde = @enable;
)]] MaterialValue
{
    EMaterialPropertyType prop_type;
    MaterialPropertyName slot_name;

    double value = 0.0;
    float4 vec = { 0.0f, 0.0f, 0.0f, 0.0f };
    SResourceHandle resource;
};

struct [[sattr(
    guid = "ed2e3476-90a3-4f2f-ac97-808f63d1eb11";
    serde = @enable;
)]] MaterialPass
{
    String pass;
    Vector<AsyncResource<ShaderCollectionResource>> shader_resources;
    Vector<EMaterialBlendMode> blend_modes;
    bool two_sided = false;
};

struct [[sattr(
    guid = "83264b35-3fde-4fff-8ee1-89abce2e445b";
    serde = @enable;
)]] MaterialTypeResource
{
    uint32_t version;
    Vector<MaterialPass> passes;
    Vector<MaterialValue> default_values;
    Vector<ShaderOptionInstance> switch_defaults;
    Vector<ShaderOptionInstance> option_defaults;
    VertexLayoutId vertex_type;
};

struct SKR_RENDERER_API MaterialTypeFactory : public ResourceFactory
{
    struct Root
    {
        SRenderDeviceId render_device = nullptr;
    };

    virtual ~MaterialTypeFactory() = default;
    float AsyncSerdeLoadFactor() override { return 1.f; }
    [[nodiscard]] static MaterialTypeFactory* Create(const Root& root);
    static void Destroy(MaterialTypeFactory* factory);
};
} // namespace skr