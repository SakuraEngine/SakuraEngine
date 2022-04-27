#pragma once
#include "platform/adaptive_types.h"
#include "stdbool.h"
#ifdef TARGET_MACOS
    #include "platform/apple/macos/window.h"
#endif
#include "cgpu/api.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#define BACK_BUFFER_WIDTH 900
#define BACK_BUFFER_HEIGHT 900

inline static bool SDLEventHandler(const SDL_Event* event, SDL_Window* window)
{
    switch (event->type)
    {
        case SDL_WINDOWEVENT:
            if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                const int32_t ResizeWidth = event->window.data1;
                const int32_t ResizeHeight = event->window.data2;
                (void)ResizeWidth;
                (void)ResizeHeight;
            }
            else if (event->window.event == SDL_WINDOWEVENT_CLOSE)
                return false;
        default:
            return true;
    }
    return true;
}

inline static void read_bytes(const char* file_name, char8_t** bytes, uint32_t* length)
{
    FILE* f = fopen(file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (char8_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void read_shader_bytes(
    const char* virtual_path, uint32_t** bytes, uint32_t* length,
    ECGpuBackend backend)
{
    char shader_file[256];
    const char* shader_path = "./../resources/shaders/";
    strcpy(shader_file, shader_path);
    strcat(shader_file, virtual_path);
    switch (backend)
    {
        case CGPU_BACKEND_VULKAN:
            strcat(shader_file, ".spv");
            break;
        case CGPU_BACKEND_D3D12:
        case CGPU_BACKEND_XBOX_D3D12:
            strcat(shader_file, ".dxil");
            break;
        default:
            break;
    }
    read_bytes(shader_file, (char8_t**)bytes, length);
}

#ifdef __cplusplus
template <typename Pipeline>
inline static void free_pipeline_and_signature(Pipeline* pipeline)
{
    auto sig = pipeline->root_signature;
    if constexpr (std::is_same_v<Pipeline*, CGpuRenderPipelineId>)
    {
        cgpu_free_render_pipeline(pipeline);
    }
    else
    {
        cgpu_free_compute_pipeline(pipeline);
    }
    cgpu_free_root_signature(sig);
}
#endif