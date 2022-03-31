struct VSOut
{
    float4 position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

static const float2 positions[6] = {
    float2(0.5, 0.5),   //RU
    float2(-0.5, -0.5), // LD
    float2(0.5, -0.5),  // RD

    float2(0.5, 0.5),  // RU
    float2(-0.5, 0.5), // LU
    float2(-0.5, -0.5) // LD
};

static const float2 uvs[6] = {
    float2(1.0, 1.0),
    float2(0.0, 0.0),
    float2(1.0, 0.0),

    float2(1.0, 1.0),
    float2(0.0, 1.0),
    float2(0.0, 0.0)
};

VSOut main(uint VertexIndex : SV_VertexID)
{
    VSOut output;
    output.position = float4(positions[VertexIndex], 0.f, 1.f);
    output.UV = uvs[VertexIndex];
    return output;
}