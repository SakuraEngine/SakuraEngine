#pragma once

struct Constants
{
    float4x4 projection_matrix;
    float4x4 clip_matrix;
    float4 base_color;
    float4 multiply_color;
    float4 screen_color;
    float4 channel_flag;
};
[[vk::push_constant]] 
ConstantBuffer<Constants> push_constants : register(b0);