#pragma pack_matrix(row_major)

struct VSOut
{
    // ignore SV_POSITION in pixel shader if we dont use it
    // float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
#ifdef VERTEX_HAS_TANGENT
    float4 tangent : TANGENT;
#endif
};

[[vk::binding(0, 1)]]
Texture2D color_texture : register(t0, space1);
[[vk::binding(0, 2)]]
SamplerState color_sampler : register(s0, space2);
[[vk::binding(1, 1)]]
Texture2D emissive_texture : register(t1, space1);
[[vk::binding(1, 2)]]
SamplerState emissive_sampler : register(s1, space2);

void main(VSOut psIn,     
    out float4 o_color : SV_Target0) : SV_TARGET
{
    float2 uv = psIn.uv;
    o_color = color_texture.Sample(color_sampler, uv) + emissive_texture.Sample(emissive_sampler, uv);
}