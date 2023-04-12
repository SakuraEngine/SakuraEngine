#pragma pack_matrix(row_major)

struct VSIn
{
    float2 uv : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    centroid float3 normal : NORMAL;
    centroid float4 tangent : TANGENT;
};

struct VSOut
{
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct ForwardRenderConstants
{
    float4x4 view_proj;
};

struct RootConstants
{
    float4x4 model;
};
[[vk::push_constant]]
ConstantBuffer<RootConstants> push_constants;

[[vk::binding(0, 0)]]
ConstantBuffer<ForwardRenderConstants> pass_cb : register(b0, space0);

VSOut main(const VSIn input, out float4 position : SV_POSITION)
{
    VSOut output;
    float4 posW = mul(float4(input.position, 1.0f), push_constants.model);
    float4 posH = mul(posW, pass_cb.view_proj);
    position = posH;
    output.uv = input.uv;
    output.normal = float4(input.normal, 0.f);
    output.tangent = input.tangent;
    return output;
}