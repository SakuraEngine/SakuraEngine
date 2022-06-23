struct VSIn
{
    float3 position : POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 UV : TEXCOORD0;
};

VSOut main(const VSIn input, uint VertexIndex : SV_VertexID)
{
    VSOut output;
    output.position = float4(input.position, 1.f);
    output.UV = input.uv;
    output.color = input.color;
    return output;
}