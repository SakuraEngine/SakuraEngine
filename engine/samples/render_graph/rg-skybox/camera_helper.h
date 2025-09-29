#pragma once
#include <SkrBase/math.hpp>

namespace utils
{
struct Camera
{
    skr::float3 pos = { 0.0f, 5.0f, -5.0f };    // camera position
    skr::float3 front = skr::float3::forward(); // camera front vector
    skr::float3 up = skr::float3::up();         // camera up vector
    skr::float3 right = skr::float3::right();   // camera right vector

    float fov = 3.1415926f / 2.f; // fov_x
    float aspect = 1.0;           // aspect ratio
    float near_plane = 0.1;       // near plane distance
    float far_plane = 1000.0;     // far plane distance
};

class CameraController
{
    utils::Camera* camera = nullptr;

public:
    void set_camera(utils::Camera* cam) { camera = cam; }
    void imgui_camera_info_frame();
    void imgui_control_frame();
};

} // namespace utils