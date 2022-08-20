#pragma once

struct Constants
{
    float4x4 projection_matrix;
    float4x4 clip_matrix;
    float4 base_color;
    float4 multiply_color;
    float4 screen_color;
    float4 channel_flag;
    float use_mask;
    float pad0;
    float pad1;
    float pad2;
};
[[vk::push_constant]] 
ConstantBuffer<Constants> push_constants : register(b0);