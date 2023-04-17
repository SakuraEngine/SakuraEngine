struct VSIn
{
    float3 position : POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct VSOut
{
    float3 color : COLOR;
    float2 UV : TEXCOORD0;
};

VSOut main(const VSIn input, uint VertexIndex : SV_VertexID,
    out float4 position : SV_POSITION)
{
    VSOut output;
    output.UV = input.uv;
    output.color = input.color;
    
    position = float4(input.position, 1.f);

    return output;
}