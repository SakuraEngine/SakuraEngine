#pragma pack_matrix(row_major)

// reverse depth: #define QUAD_Z 1
#define QUAD_Z 0

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
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
	vout.uv = uvs[VertexIndex];
	vout.position = float4(vout.uv.x * 2.f - 1.f, vout.uv.y * 2.f - 1.f, QUAD_Z, 1.f);
    return vout;
}