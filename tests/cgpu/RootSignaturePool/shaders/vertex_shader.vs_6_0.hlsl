struct VSOut
{
    float4 position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

static const float2 positions[6] = {
    float2(0.5, 0.5),   //RU   0
    float2(-0.5, -0.5), // LD  1
    float2(0.5, -0.5),  // RD  2

    float2(0.5, 0.5),  // RU   3
    float2(-0.5, 0.5), // LU   4
    float2(-0.5, -0.5) // LD   5
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