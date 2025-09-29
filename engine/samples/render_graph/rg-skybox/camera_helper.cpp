#include "camera_helper.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRenderer/render_mesh.h"
#include <imgui.h>

namespace utils
{

void CameraController::imgui_camera_info_frame()
{
    ImGui::Begin("Camera Info");
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera->pos.x, camera->pos.y, camera->pos.z);
    ImGui::Text("Front:    (%.2f, %.2f, %.2f)", camera->front.x, camera->front.y, camera->front.z);
    ImGui::End();
}

void CameraController::imgui_control_frame()
{
    constexpr float kPi = rtm::constants::pi();
    ImGuiIO& io = ImGui::GetIO();

    const float camera_speed = 0.01f * io.DeltaTime;
    const float camera_pan_speed = 0.0025f * io.DeltaTime;
    const float camera_sensitivity = 0.05f;

    skr::float3 world_up = { 0.0f, 1.0f, 0.0f };
    static float yaw = 90.0f;
    static float pitch = -45.0f;
    // Rotation
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        if (ImGui::IsKeyDown(ImGuiKey_W)) camera->pos += camera->front * camera_speed;
        if (ImGui::IsKeyDown(ImGuiKey_S)) camera->pos -= camera->front * camera_speed;
        if (ImGui::IsKeyDown(ImGuiKey_A)) camera->pos -= camera->right * camera_speed;
        if (ImGui::IsKeyDown(ImGuiKey_D)) camera->pos += camera->right * camera_speed;

        yaw += io.MouseDelta.x * camera_sensitivity;
        pitch -= io.MouseDelta.y * camera_sensitivity;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    skr::float3 direction;
    direction.x = cosf(yaw * (float)kPi / 180.f) * cosf(pitch * (float)kPi / 180.f);
    direction.y = sinf(pitch * (float)kPi / 180.f);
    direction.z = sinf(yaw * (float)kPi / 180.f) * cosf(pitch * (float)kPi / 180.f);
    camera->front = skr::normalize(direction);

    // Panning
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
    {
        camera->pos -= camera->right * io.MouseDelta.x * camera_pan_speed;
        camera->pos += camera->up * io.MouseDelta.y * camera_pan_speed;
    }
    imgui_camera_info_frame();
}

} // namespace utils