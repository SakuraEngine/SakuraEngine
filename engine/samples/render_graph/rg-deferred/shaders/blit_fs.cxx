#include <std/std.hxx>

[[group(0)]]
Texture2D input_color;

[[group(1)]]
Sampler texture_sampler;

struct [[stage_inout]] VSOut
{
    float2 uv;
};

[[fragment_shader("fs")]]
void fs(VSOut psIn, [[sv_render_target(0)]] float4 o_color)
{
    float2 uv = psIn.uv;
    o_color = input_color.Sample(texture_sampler, uv);
}