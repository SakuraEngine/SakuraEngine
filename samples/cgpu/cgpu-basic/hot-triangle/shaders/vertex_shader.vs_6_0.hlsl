struct VSOut
{
    float4 position : SV_POSITION;
    float3 color : TEXCOORD0;
};

static const float2 positions[3] = {
    float2(0.0, 0.5),
    float2(-0.5, -0.5),
    float2(0.5, -0.5)
};

static const float3 colors[3] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
};

VSOut main(uint VertexIndex : SV_VertexID)
{
    VSOut output;
    output.position = float4(positions[VertexIndex], 0.f, 1.f);
    output.color = colors[VertexIndex];
    return output;
}