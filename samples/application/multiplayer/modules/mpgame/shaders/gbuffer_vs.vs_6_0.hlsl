#pragma pack_matrix(row_major)

struct VSIn
{
    centroid float3 normal : NORMAL;
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
};

struct RootConstants
{
    float4x4 model;
    float4x4 view_proj;
};
[[vk::push_constant]]
ConstantBuffer<RootConstants> push_constants : register(b0);

VSOut main(const VSIn input)
{
    VSOut output;
    float4 posW = mul(float4(input.position, 1.0f), push_constants.model);
    float4 posH = mul(posW, push_constants.view_proj);
    output.position = posH;
    output.uv = input.uv * push_constants.model[0][0];
    output.normal = float4(input.normal, 0.f);
    return output;
}