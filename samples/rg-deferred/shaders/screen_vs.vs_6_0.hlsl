// reverse depth: #define QUAD_Z 1
#define QUAD_Z 0

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

static const float2 positions[6] = {
    float2(1.f, 1.f),   //RU   0
    float2(-1.f, -1.f), // LD  1
    float2(1.f, -1.f),  // RD  2

    float2(1.f, 1.f),  // RU   3
    float2(-1.f, 1.f), // LU   4
    float2(-1.f, -1.f) // LD   5
};

static const float2 uvs[6] = {
    float2(1.0, 1.0),
    float2(0.0, 0.0),
    float2(1.0, 0.0),

    float2(1.0, 1.0),
    float2(0.0, 1.0),
    float2(0.0, 0.0)
};


VSOut main(in uint VertexIndex : SV_VertexID)
{
    VSOut vout;
	vout.position = float4(positions[VertexIndex], 0.f, 1.f);
	vout.uv = uvs[VertexIndex];
    return vout;
}