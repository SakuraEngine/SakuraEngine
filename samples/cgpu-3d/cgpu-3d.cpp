#include "render-device.hpp"
#include "render-resources.hpp"
#include "render-context.hpp"
#include "render-scene.hpp"
#include "math/scalarmath.h"
#include "math/vectormath.hpp"
#include <EASTL/unique_ptr.h>
#include <EASTL/string.h>

namespace smath = sakura::math;

typedef struct PushConstants {
    smath::float4x4 world;
    smath::float4x4 view_proj;
} PushConstants;

static PushConstants data = {};

int main(int argc, char* argv[])
{
    eastl::string cmdArgv1 = argv[0];
    ECGpuBackend cmdBackend = cmdArgv1 == "-dx12" ? CGPU_BACKEND_D3D12 : CGPU_BACKEND_VULKAN;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    auto render_device = eastl::make_unique<RenderDevice>();
    RenderWindow* render_window = nullptr;
    render_device->Initialize(cmdBackend, &render_window);

    auto aux_thread = eastl::make_unique<RenderAuxThread>();
    aux_thread->Initialize(render_device.get());

    auto render_context = eastl::make_unique<RenderContext>();
    render_context->Initialize(render_device.get());
    auto render_scene = eastl::make_unique<RenderScene>();
    render_scene->Initialize("./../Resources/scene.gltf");
    render_scene->Upload(render_context.get(), aux_thread.get());
    // wvp
    auto world = smath::make_transform(
        { 0.f, 0.f, 0.f },                                             // translation
        sakura::math::Vector3f::vector_one(),                          // scale
        render_scene->nodes_[render_scene->root_node_index_].rotation_ // quat
    );
    auto view = smath::look_at_matrix({ 0.f, 55.f, 137.5f } /*eye*/, { 0.f, 50.f, 0.f } /*at*/);
    auto proj = smath::perspective_fov(
        3.1415926f / 2.f,
        (float)BACK_BUFFER_HEIGHT / (float)BACK_BUFFER_WIDTH,
        1.f, 1000.f);
    proj.M[3][2] *= -1;
    data.world = smath::transpose(world);
    data.view_proj = smath::transpose(smath::multiply(view, proj));
    // render loop
    bool quit = false;
    uint32_t backbuffer_index = 0;
    auto present_semaphore = render_device->AllocSemaphore();
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(render_window->sdl_window_) == event.window.windowID)
            {
                quit = !SDLEventHandler(&event, render_window->sdl_window_);
            }
        }
        CGpuAcquireNextDescriptor acquire_desc = {};
        acquire_desc.signal_semaphore = present_semaphore;
        backbuffer_index = render_device->AcquireNextFrame(render_window, acquire_desc);
        const CGpuTextureId back_buffer = render_window->swapchain_->back_buffers[backbuffer_index];
        render_context->Wait();
        render_context->Begin();
        // render pass
        CGpuColorAttachment screen_attachment = {};
        screen_attachment.view = render_window->views_[backbuffer_index];
        screen_attachment.load_action = LOAD_ACTION_CLEAR;
        screen_attachment.store_action = STORE_ACTION_STORE;
        screen_attachment.clear_color = fastclear_0000;
        CGpuDepthStencilAttachment ds_attachment = {};
        ds_attachment.view = render_window->screen_ds_view_[backbuffer_index];
        ds_attachment.write_depth = true;
        ds_attachment.clear_depth = true;
        ds_attachment.write_stencil = false;
        ds_attachment.depth_load_action = LOAD_ACTION_LOAD;
        ds_attachment.depth_store_action = STORE_ACTION_STORE;
        CGpuRenderPassDescriptor rp_desc = {};
        rp_desc.render_target_count = 1;
        rp_desc.sample_count = SAMPLE_COUNT_1;
        rp_desc.color_attachments = &screen_attachment;
        rp_desc.depth_stencil = &ds_attachment;
        CGpuTextureBarrier draw_barrier = {};
        draw_barrier.texture = back_buffer;
        draw_barrier.src_state = RESOURCE_STATE_UNDEFINED;
        draw_barrier.dst_state = RESOURCE_STATE_RENDER_TARGET;
        CGpuResourceBarrierDescriptor barrier_desc0 = {};
        barrier_desc0.texture_barriers = &draw_barrier;
        barrier_desc0.texture_barriers_count = 1;
        render_context->ResourceBarrier(barrier_desc0);
        {
            render_context->BeginRenderPass(rp_desc);
            render_context->SetViewport(0.0f, 0.0f,
                (float)back_buffer->width, (float)back_buffer->height,
                0.f, 1.f);
            render_context->SetScissor(0, 0, back_buffer->width, back_buffer->height);
            // foreach prim
            CGpuRenderPipelineId cached_pipeline = nullptr;
            CGpuDescriptorSetId cached_descset = nullptr;
            for (uint32_t i = 0; i < render_scene->meshes_.size(); i++)
            {
                auto& mesh = render_scene->meshes_[i];
                for (uint32_t j = 0; j < mesh.primitives_.size(); j++)
                {
                    auto& prim = mesh.primitives_[j];
                    if (cached_pipeline != prim.pipeline_)
                    {
                        render_context->BindPipeline(prim.pipeline_);
                        cached_pipeline = prim.pipeline_;
                    }
                    if (cached_descset != prim.desc_set_)
                    {
                        render_context->BindDescriptorSet(prim.desc_set_);
                        cached_descset = prim.desc_set_;
                    }
                    render_scene->index_buffer_.Wait();
                    render_context->BindIndexBuffer(
                        render_scene->index_buffer_.buffer_, render_scene->index_stride_, prim.index_offset_);
                    const uint32_t vbc = prim.vertex_buffers_.size();
                    render_context->BindVertexBuffers(
                        vbc,
                        prim.vertex_buffers_.data(),
                        prim.vertex_strides_.data(),
                        prim.vertex_offsets_.data());
                    render_context->PushConstants(prim.pipeline_->root_signature, "root_constants", &data);
                    render_context->DrawIndexedInstanced(prim.index_count_, prim.first_index_, 1, 0, 0);
                }
            }
            render_context->EndRenderPass();
        }
        CGpuTextureBarrier present_barrier = {};
        present_barrier.texture = back_buffer;
        present_barrier.src_state = RESOURCE_STATE_RENDER_TARGET;
        present_barrier.dst_state = RESOURCE_STATE_PRESENT;
        CGpuResourceBarrierDescriptor barrier_desc1 = {};
        barrier_desc1.texture_barriers = &present_barrier;
        barrier_desc1.texture_barriers_count = 1;
        render_context->ResourceBarrier(barrier_desc1);
        render_context->End();
        render_device->Submit(render_context.get());
        render_device->Present(render_window, backbuffer_index, &present_semaphore, 1);
        render_device->CollectGarbage();
    }
    render_device->FreeSemaphore(present_semaphore);
    render_device->WaitIdle();
    render_scene->Destroy(aux_thread.get());
    render_context->Destroy();
    render_window->Destroy();
    aux_thread->Destroy();
    render_device->Destroy();
    return 0;
}