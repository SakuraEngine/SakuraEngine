#include <std/std.hxx>



struct RootConstants
{
    uint TextureIndex;      // Which texture to use from the bindless array
    float ColorMultiplier;
};

// Bindless texture array - length 0 means unbounded/bindless
Texture2D<> sampled_textures[0];

[[group(1)]]
Sampler texture_sampler;

[[push_constant]]
ConstantBuffer<RootConstants> push_constants;

struct [[stage_inout]] VSOut
{
    float2 uv;
};

static const float2 positions[6] = {
    float2(0.5, 0.5),   // RU  0
    float2(-0.5, -0.5), // LD  1
    float2(0.5, -0.5),  // RD  2
    
    float2(0.5, 0.5),   // RU  3
    float2(-0.5, 0.5),  // LU  4
    float2(-0.5, -0.5)  // LD  5
};

static const float2 uvs[6] = {
    float2(1.0, 1.0),
    float2(0.0, 0.0),
    float2(1.0, 0.0),
    
    float2(1.0, 1.0),
    float2(0.0, 1.0),
    float2(0.0, 0.0)
};

[[vertex_shader("vs")]]
VSOut vertex([[sv_vertex_id]] uint VertexIndex, [[sv_position]] float4& position)
{
    VSOut output;
    output.uv = uvs[VertexIndex];
    position = float4(positions[VertexIndex], 0.f, 1.f);
    return output;
}

[[fragment_shader("fs")]]
void fragment(VSOut psIn, [[sv_render_target(0)]] float4& color) 
{
    // Access texture from bindless array using dynamic index
    uint texture_index = push_constants.TextureIndex;
    float4 sampled_color = sampled_textures[texture_index].Sample(texture_sampler, psIn.uv);
    
    // Apply color multiplier
    color = sampled_color * push_constants.ColorMultiplier;
}