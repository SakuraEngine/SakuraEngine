#pragma pack_matrix(row_major)

// reverse depth: #define QUAD_Z 1
#define QUAD_Z 0

struct VSOut
{
    float2 uv : TEXCOORD0;
};

static const float2 uvs[3] = {
    float2(2.f, 1.f),
    float2(0.f, -1.f),
    float2(0.f, 1.f)
};

static const float2 poses[3] = {
    float2(3.f, -1.f),
    float2(-1.f, 3.f),
    float2(-1.f, -1.f)
};

VSOut main(in uint VertexIndex : SV_VertexID, out float4 position : SV_POSITION)
{
    VSOut vout;
	vout.uv = uvs[VertexIndex];
	position = float4(poses[VertexIndex], QUAD_Z, 1.f);
    return vout;
}