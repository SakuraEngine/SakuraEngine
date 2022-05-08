#include "render-device.hpp"
#include "render-resources.hpp"
#include "render-context.hpp"
#include "render-scene.hpp"
#include "math/scalarmath.h"
#include "math/vectormath.hpp"
#include <EASTL/unique_ptr.h>
#include <EASTL/string.h>

namespace smath = skr::math;

typedef struct PushConstants {
    smath::float4x4 world;
    smath::float4x4 view_proj;
} PushConstants;

static PushConstants data = {};

template <typename T, typename... Args>
eastl::unique_ptr<T> CreateAndInitialize(Args&&... args)
{
    auto result = eastl::make_unique<T>();
    result->Initialize(eastl::forward<Args>(args)...);
    return eastl::move(result);
}

#define TEXTURE_WIDTH 2
#define TEXTURE_HEIGHT 2
const uint8_t TEXTURE_DATA[] = {
    0, 0, 0, 0,
    255, 255, 255, 255,
    255, 255, 255, 255,
    0, 0, 0, 0
};

#define MAX_FLIGHT_FRAMES 3
class SceneRenderer
{
public:
    FORCEINLINE void Initialize(RenderDevice* render_device)
    {
        render_device_ = render_device;
        RenderBlackboard::Initialize();
        // we can fork multiple threads to create resources of different types
        // here we create two aux_threads for memory_resource & pso creation
        aux_thread = CreateAndInitialize<RenderAuxThread>(render_device);
        pso_aux_thread = CreateAndInitialize<RenderAuxThread>(render_device);
        async_transfer_thread = CreateAndInitialize<AsyncTransferThread>(render_device);
        // context & default resources
        for (uint32_t i = 0; i < MAX_FLIGHT_FRAMES; i++)
        {
            render_contexts_[i] = CreateAndInitialize<RenderContext>(render_device);
        }
        auto target = RenderBlackboard::AddTexture("DefaultTexture", aux_thread.get(), TEXTURE_WIDTH, TEXTURE_HEIGHT);
        auto defaultTexUploadFence = render_device->AllocFence();
        target->Wait();
        async_transfer_thread->UploadTexture(target, TEXTURE_DATA, sizeof(TEXTURE_DATA), defaultTexUploadFence);
        cgpu_wait_fences(&defaultTexUploadFence, 1);
        render_contexts_[0]->Begin();
        render_contexts_[0]->AcquireResources(&target, 1, nullptr, 0);
        render_contexts_[0]->End();
        render_device->Submit(render_contexts_[0].get());
    }
    FORCEINLINE void PrepareRenderResourceMemory(RenderScene* render_scene)
    {
        render_scene->AsyncCreateGeometryMemory(render_device_, aux_thread.get());
        render_scene->AsyncCreateTextureMemory(render_device_, aux_thread.get());
        render_scene->AsyncCreateRenderPipelines(render_device_, pso_aux_thread.get());
    }
    FORCEINLINE void DrawToScreen(RenderWindow* render_window, RenderScene* render_scene)
    {
        auto render_context = render_contexts_[context_index].get();
        render_context->Wait();
        render_context->Begin();
        // try upload resources
        {
            render_scene->TryAsyncUploadBuffers(render_context, async_transfer_thread.get());
            render_scene->TryAsyncUploadTextures(render_context, async_transfer_thread.get());
        }
        // render pass
        render_window->BeginScreenPass(render_context);
        // begin render scene
        if (render_scene->AsyncGeometryUploadReady())
        {
            for (uint32_t i = 0; i < render_scene->meshes_.size(); i++)
            {
                auto& mesh = render_scene->meshes_[i];
                for (uint32_t j = 0; j < mesh.primitives_.size(); j++)
                {
                    auto& prim = mesh.primitives_[j];
                    auto prim_pipeline = prim.async_ppl_;
                    if (prim_pipeline->Ready())
                    {
                        render_context->BindPipeline(prim_pipeline->pipeline_);
                        render_context->BindDescriptorSet(
                        prim.desc_set_updated_ ? prim.desc_set_ : prim_pipeline->desc_set_);
                        render_context->BindIndexBuffer(render_scene->index_buffer_.buffer_,
                        render_scene->index_stride_, prim.index_offset_);
                        const auto vbc = (uint32_t)prim.vertex_buffers_.size();
                        render_context->BindVertexBuffers(
                        vbc,
                        prim.vertex_buffers_.data(),
                        prim.vertex_strides_.data(),
                        prim.vertex_offsets_.data());
                        render_context->PushConstants(prim_pipeline->pipeline_->root_signature, "root_constants", &data);
                        render_context->DrawIndexedInstanced(prim.index_count_, prim.first_index_, 1, 0, 0);
                    }
                }
            }
        }
        render_window->EndScreenPass(render_context);
        render_context->End();
        render_device_->Submit(render_context);
        context_index = (context_index + 1) % MAX_FLIGHT_FRAMES;
    }
    FORCEINLINE void Destroy()
    {
        RenderBlackboard::Finalize();
        // stop & wait cpy cmds
        aux_thread->Destroy();
        async_transfer_thread->Destroy();
        pso_aux_thread->Destroy();
        // destroy driver objects
        for (uint32_t i = 0; i < MAX_FLIGHT_FRAMES; i++)
            render_contexts_[i]->Destroy();
    }

protected:
    uint32_t context_index = 0;
    RenderDevice* render_device_;
    eastl::unique_ptr<RenderContext> render_contexts_[MAX_FLIGHT_FRAMES];
    eastl::unique_ptr<RenderAuxThread> aux_thread;
    eastl::unique_ptr<RenderAuxThread> pso_aux_thread;
    eastl::unique_ptr<AsyncTransferThread> async_transfer_thread;
};

int main(int argc, char* argv[])
{
    ECGPUBackend cmdBackend;
    if (argc <= 1)
        cmdBackend = CGPU_BACKEND_VULKAN;
    else
    {
        eastl::string cmdArgv1 = argv[1];
        cmdBackend = cmdArgv1 == "-dx12" ? CGPU_BACKEND_D3D12 : CGPU_BACKEND_VULKAN;
    }
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    RenderWindow* render_window = nullptr;
    auto render_device = CreateAndInitialize<RenderDevice>(cmdBackend, &render_window);
    auto renderer = CreateAndInitialize<SceneRenderer>(render_device.get());
    // scene
    auto render_scene = CreateAndInitialize<RenderScene>("./../resources/scene.gltf");
    renderer->PrepareRenderResourceMemory(render_scene.get());
    // wvp
    auto world = smath::make_transform(
    { 0.f, 0.f, 0.f },                                             // translation
    skr::math::Vector3f::vector_one(),                             // scale
    render_scene->nodes_[render_scene->root_node_index_].rotation_ // quat
    );
    // camera
    auto view = smath::look_at_matrix({ 0.f, 55.f, 137.5f } /*eye*/, { 0.f, 50.f, 0.f } /*at*/);
    auto proj = smath::perspective_fov(
    3.1415926f / 2.f,
    (float)BACK_BUFFER_HEIGHT / (float)BACK_BUFFER_WIDTH,
    1.f, 1000.f);
    data.world = smath::transpose(world);
    data.view_proj = smath::transpose(smath::multiply(view, proj));
    // render loop
    bool quit = false;
    uint32_t backbuffer_index = 0;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(render_window->GetSDLWindow()) == event.window.windowID)
            {
                quit = !SDLEventHandler(&event, render_window->GetSDLWindow());
            }
        }
        // swapchain get ready
        render_window->AcquireNextFrame(backbuffer_index);
        renderer->DrawToScreen(render_window, render_scene.get());
        render_window->Present(backbuffer_index);
    }
    render_device->WaitIdle();
    // destroy render resources
    render_scene->Destroy();
    renderer->Destroy();
    render_window->Destroy();
    render_device->Destroy();
    return 0;
}