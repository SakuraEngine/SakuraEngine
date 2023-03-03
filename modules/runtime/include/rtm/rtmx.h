#pragma once
#include "rtm/matrix4x4f.h"
#include "rtm/mask4i.h"

// RH
namespace rtm
{
constexpr uint32_t RTM_SELECT_0 = 0x00000000;
constexpr uint32_t RTM_SELECT_1 = ~RTM_SELECT_0;
static const rtm::mask4f RTM_SELECT_1110F = rtm::mask_set(RTM_SELECT_1, RTM_SELECT_1, RTM_SELECT_1, RTM_SELECT_0);

RTM_DISABLE_SECURITY_COOKIE_CHECK inline 
rtm::matrix4x4f RTM_SIMD_CALL orthographic(float ViewWidth, float ViewHeight, float NearZ, float FarZ) RTM_NO_EXCEPT
{
    float fRange = 1.0f / (NearZ - FarZ);

    const auto col0 = rtm::vector_set(2.0f / ViewWidth, 0.f, 0.f, 0.f);
    const auto col1 = rtm::vector_set(0.f, 2.0f / ViewHeight, 0.f, 0.f);
    const auto col2 = rtm::vector_set(0.f, 0.f, fRange, 0.f);
    const auto col3 = rtm::vector_set(0.f, 0.f, fRange * NearZ, 1.f);

    return rtm::matrix_set(col0, col1, col2, col3);
}

RTM_DISABLE_SECURITY_COOKIE_CHECK inline 
rtm::matrix4x4f RTM_SIMD_CALL orthographic_lh(float ViewWidth, float ViewHeight, float NearZ, float FarZ) RTM_NO_EXCEPT
{
    float fRange = 1.0f / (FarZ - NearZ);

    const auto col0 = rtm::vector_set(2.0f / ViewWidth, 0.f, 0.f, 0.f);
    const auto col1 = rtm::vector_set(0.f, 2.0f / ViewHeight, 0.f, 0.f);
    const auto col2 = rtm::vector_set(0.f, 0.f, fRange, 0.f);
    const auto col3 = rtm::vector_set(0.f, 0.f, -fRange * NearZ, 1.f);

    return rtm::matrix_set(col0, col1, col2, col3);
}

RTM_DISABLE_SECURITY_COOKIE_CHECK inline 
rtm::matrix4x4f RTM_SIMD_CALL perspective_fov(float FovAngleY, float AspectRatio, float NearZ, float FarZ) RTM_NO_EXCEPT
{
    float    SinFov;
    float    CosFov;
    rtm::scalar_sincos(0.5f * FovAngleY, SinFov, CosFov);

    const float Height = CosFov / SinFov;
    const float Width = Height / AspectRatio;
    const float fRange = FarZ / (NearZ - FarZ);

    const auto col0 = rtm::vector_set(Width, 0.f, 0.f, 0.f);
    const auto col1 = rtm::vector_set(0.f, Height, 0.f, 0.f);
    const auto col2 = rtm::vector_set(0.f, 0.f, fRange, -1.f);
    const auto col3 = rtm::vector_set(0.f, 0.f, fRange * NearZ, 0.f);

    return rtm::matrix_set(col0, col1, col2, col3);
}

RTM_DISABLE_SECURITY_COOKIE_CHECK inline 
rtm::matrix4x4f RTM_SIMD_CALL look_to_matrix_lh(rtm::vector4f_arg0 EyePosition, rtm::vector4f_arg1 EyeDirection, rtm::vector4f_arg2 UpDirection) RTM_NO_EXCEPT
{
    const auto R2 = rtm::vector_normalize3(EyeDirection);
    const auto R0 = rtm::vector_normalize3( rtm::vector_cross3(UpDirection, R2) );

    const auto R1 = rtm::vector_cross3(R2, R0);
    const auto NegEyePosition = rtm::vector_neg(EyePosition);

    const rtm::vector4f D0 = ( rtm::vector_dot3(R0, NegEyePosition) );
    const rtm::vector4f D1 = ( rtm::vector_dot3(R1, NegEyePosition) );
    const rtm::vector4f D2 = ( rtm::vector_dot3(R2, NegEyePosition) );

    const rtm::vector4f col0 = rtm::vector_select(RTM_SELECT_1110F, R0, D0);
    const rtm::vector4f col1 = rtm::vector_select(RTM_SELECT_1110F, R1, D1);
    const rtm::vector4f col2 = rtm::vector_select(RTM_SELECT_1110F, R2, D2);
    const rtm::vector4f col3 = rtm::vector_set(0.f, 0.f, 0.f, 1.f);
    
    const matrix4x4f mat4x4 = rtm::matrix_set(col0, col1, col2, col3);
    const matrix4x4f result = rtm::matrix_transpose(mat4x4);
    return result;
}

RTM_DISABLE_SECURITY_COOKIE_CHECK inline 
rtm::matrix4x4f RTM_SIMD_CALL look_to_matrix(rtm::vector4f_arg0 EyePosition, rtm::vector4f_arg1 EyeDirection, rtm::vector4f_arg2 UpDirection) RTM_NO_EXCEPT
{
    const auto NegEyeDirection = rtm::vector_neg(EyeDirection);
    return look_to_matrix_lh(EyePosition, NegEyeDirection, UpDirection);
}

RTM_DISABLE_SECURITY_COOKIE_CHECK inline 
rtm::matrix4x4f RTM_SIMD_CALL look_at_matrix_lh(rtm::vector4f_arg0 EyePosition, rtm::vector4f_arg1 FocusPosition, rtm::vector4f_arg2 UpDirection) RTM_NO_EXCEPT
{
    const auto EyeDirection = rtm::vector_sub(FocusPosition, EyePosition);
    return look_to_matrix_lh(EyePosition, EyeDirection, UpDirection);
}

RTM_DISABLE_SECURITY_COOKIE_CHECK inline 
rtm::matrix4x4f RTM_SIMD_CALL look_at_matrix(rtm::vector4f_arg0 EyePosition, rtm::vector4f_arg1 FocusPosition, rtm::vector4f_arg2 UpDirection) RTM_NO_EXCEPT
{
    const auto NegEyeDirection = rtm::vector_sub(EyePosition, FocusPosition);
    return look_to_matrix_lh(EyePosition, NegEyeDirection, UpDirection);
}

RTM_DISABLE_SECURITY_COOKIE_CHECK inline
uint32_t RTM_SIMD_CALL vector_to_snorm8(vector4f_arg0 v)
{
    const auto _x = rtm::vector_get_x(v);
    const auto _y = rtm::vector_get_y(v);
    const auto _z = rtm::vector_get_z(v);
    const auto _w = rtm::vector_get_w(v);

    const float scale = 127.0f / sqrtf(_x * _x + _y * _y + _z * _z);
    const uint32_t x = int(_x * scale);
    const uint32_t y = int(_y * scale);
    const uint32_t z = int(_z * scale);
    const uint32_t w = int(_w * scale);
    return (x & 0xff) | ((y & 0xff) << 8) | ((z & 0xff) << 16) | ((w & 0xff) << 24);
}

}
