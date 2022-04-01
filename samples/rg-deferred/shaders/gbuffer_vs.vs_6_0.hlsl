struct VSIn
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
    [[vk::location(2)]] centroid float4 normal : NORMAL;
    [[vk::location(3)]] centroid float4 tangent : TANGENT;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
    centroid float4 tangent : TANGENT;
};

struct RootConstants
{
    float4x4 world;
    float4x4 view_proj;
};

[[vk::push_constant]]
ConstantBuffer<RootConstants> root_constants : register(b0);

VSOut main(const VSIn input, uint VertexIndex : SV_VertexID)
{
    VSOut output;
    float4 posW = mul(float4(input.position, 1.0f), root_constants.world);
    float4 posH = mul(posW, root_constants.view_proj);
    output.position = posH;
    output.uv = input.uv;
    output.normal = input.normal;
    output.tangent = input.tangent;
    return output;
}