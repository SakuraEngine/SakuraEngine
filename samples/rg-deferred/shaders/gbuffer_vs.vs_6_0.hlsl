struct VSIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
    centroid float4 tangent : TANGENT;
    nointerpolation float4x4 model : MODEL;
    nointerpolation float4x4 view_proj : VIEWPROJ;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
    centroid float4 tangent : TANGENT;
};

VSOut main(const VSIn input)
{
    VSOut output;
    float4 posW = mul(float4(input.position, 1.0f), input.model);
    float4 posH = mul(posW, input.view_proj);
    output.position = posH;
    output.uv = input.uv;
    output.normal = input.normal;
    output.tangent = input.tangent;
    return output;
}