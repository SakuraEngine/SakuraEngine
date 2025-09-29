#include <std/std.hxx>



struct [[stage_inout]] VSOut
{
    float2 uv;
};

[[vertex_shader("vs")]]
VSOut vs([[sv_vertex_id]] uint VertexIndex, [[sv_position]] float4& position)
{
    const float QUAD_Z = 0.f; // Z position for the quad vertices
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

    VSOut vout;
	vout.uv = uvs[VertexIndex];
	position = float4(poses[VertexIndex], QUAD_Z, 1.f);
    return vout;
}