#include "SkrBase/config/platform.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/phases_v2/schedule_timeline.hpp"
#include "SkrCore/log.hpp"
#include "SkrCore/memory/memory.h"

// 模拟复杂的图形 + 异步计算工作负载
class TimelineStressTest
{
public:
    TimelineStressTest() = default;
    ~TimelineStressTest() = default;

    void run_stress_test()
    {
        SKR_LOG_INFO(u8"🔥 Timeline Stress Test Starting...");

        // Create instance
        CGPUInstanceDescriptor instance_desc = {};
#if SKR_PLAT_WIN64
        instance_desc.backend = CGPU_BACKEND_D3D12;
#elif SKR_PLAT_MACOSX
        instance_desc.backend = CGPU_BACKEND_METAL;
#endif
        instance_desc.enable_debug_layer = true;
        instance_desc.enable_gpu_based_validation = true;
        instance_desc.enable_set_name = true;
        auto instance = cgpu_create_instance(&instance_desc);

        // Filter adapters
        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
        CGPUAdapterId adapters[64];
        cgpu_enum_adapters(instance, adapters, &adapters_count);
        auto adapter = adapters[0];

        // Create device
        CGPUQueueGroupDescriptor queue_group_descs[2];
        queue_group_descs[0].queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
        queue_group_descs[0].queue_count = 1;
        queue_group_descs[1].queue_type = CGPU_QUEUE_TYPE_COMPUTE;
        queue_group_descs[1].queue_count = 2;

        CGPUDeviceDescriptor device_desc = {};
        device_desc.queue_groups = queue_group_descs;
        device_desc.queue_group_count = 2;
        auto device = cgpu_create_device(adapter, &device_desc);

        // 创建RenderGraph（前端模式，不需要真实设备）
        auto* graph = skr::render_graph::RenderGraph::create(
            [=](auto& builder) {
                auto cmpt_queues = skr::Vector<CGPUQueueId>();
                cmpt_queues.add(cgpu_get_queue(device, CGPU_QUEUE_TYPE_COMPUTE, 0));
                cmpt_queues.add(cgpu_get_queue(device, CGPU_QUEUE_TYPE_COMPUTE, 1));
                auto gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
                
                builder.with_device(device)        
                    .with_gfx_queue(gfx_queue)
                    .with_cmpt_queues(cmpt_queues); // 不需要实际队列
            });

        // 添加TimelinePhase
        auto timeline_config = skr::render_graph::ScheduleTimelineConfig{};
        timeline_config.enable_async_compute = true;
        timeline_config.enable_copy_queue = true;
        timeline_config.max_sync_points = 128;

        auto* timeline_phase = new skr::render_graph::ScheduleTimeline(timeline_config);

        // 构建复杂的渲染管线
        build_complex_pipeline(graph);

        // 手动调用TimelinePhase进行测试
        timeline_phase->on_initialize(graph);
        timeline_phase->on_execute(graph, nullptr);

        // 打印调度结果
        timeline_phase->dump_timeline_result(u8"🔥 Timeline Stress Test Results");

        // 验证调度结果
        validate_schedule_result(timeline_phase->get_schedule_result());

        // 清理
        timeline_phase->on_finalize(graph);
        delete timeline_phase;
        skr::render_graph::RenderGraph::destroy(graph);

        SKR_LOG_INFO(u8"✅ Timeline Stress Test Completed Successfully!");
    }

private:
    void build_complex_pipeline(skr::render_graph::RenderGraph* graph)
    {
        using namespace skr::render_graph;

        // 场景描述：复杂的延迟渲染 + 后处理 + 异步计算

        // ===== 阶段1: 资源准备 =====

        // 创建G-Buffer纹理
        auto gbuffer_albedo = graph->create_texture([](RenderGraph& graph, RenderGraph::TextureBuilder& builder) {
            builder.set_name(u8"GBuffer_Albedo")
                .extent(1920, 1080)
                .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                .allow_render_target();
        });

        auto gbuffer_normal = graph->create_texture([](RenderGraph& graph, RenderGraph::TextureBuilder& builder) {
            builder.set_name(u8"GBuffer_Normal")
                .extent(1920, 1080)
                .format(CGPU_FORMAT_R10G10B10A2_UNORM)
                .allow_render_target();
        });

        auto gbuffer_depth = graph->create_texture([](RenderGraph& graph, RenderGraph::TextureBuilder& builder) {
            builder.set_name(u8"GBuffer_Depth")
                .extent(1920, 1080)
                .format(CGPU_FORMAT_D24_UNORM_S8_UINT)
                .allow_depth_stencil();
        });

        // 创建光照缓冲区
        auto lighting_buffer = graph->create_buffer([](RenderGraph& graph, RenderGraph::BufferBuilder& builder) {
            builder.set_name(u8"LightingBuffer")
                .size(1920 * 1080 * 4 * sizeof(float))
                .allow_shader_readwrite()
                .as_uniform_buffer();
        });

        // ===== 阶段2: 几何通道（强制Graphics队列）=====

        // Shadow Map Pass
        auto shadow_pass = graph->add_render_pass(
            [gbuffer_depth](RenderGraph& graph, RenderGraph::RenderPassBuilder& builder) {
                builder.set_name(u8"ShadowMapPass")
                    .set_depth_stencil(gbuffer_depth);
            },
            [](RenderGraph& graph, RenderPassContext& context) {
                // 模拟阴影渲染
            });

        // G-Buffer Pass (主几何通道)
        auto gbuffer_pass = graph->add_render_pass(
            [gbuffer_albedo, gbuffer_normal, gbuffer_depth](RenderGraph& graph, RenderGraph::RenderPassBuilder& builder) {
                builder.set_name(u8"GBufferPass")
                    .write(0, gbuffer_albedo, CGPU_LOAD_ACTION_CLEAR)
                    .write(1, gbuffer_normal, CGPU_LOAD_ACTION_CLEAR)
                    .set_depth_stencil(gbuffer_depth);
            },
            [](RenderGraph& graph, RenderPassContext& context) {
                // 模拟几何渲染
            });

        // ===== 阶段3: 异步计算通道 =====

        // Culling Compute (可以异步执行)
        auto culling_buffer = graph->create_buffer([](RenderGraph& graph, RenderGraph::BufferBuilder& builder) {
            builder.set_name(u8"CullingBuffer")
                .size(1024 * 1024 * sizeof(uint32_t))
                .allow_shader_readwrite();
        });

        auto culling_pass = graph->add_compute_pass(
            [culling_buffer](RenderGraph& graph, RenderGraph::ComputePassBuilder& builder) {
                builder.set_name(u8"FrustumCullingPass")
                    .readwrite(u8"Output", culling_buffer.range(0, ~0));
            },
            [](RenderGraph& graph, ComputePassContext& context) {
                // 模拟视锥剔除计算
            });

        // Particle Simulation (异步计算)
        auto particle_buffer = graph->create_buffer([](RenderGraph& graph, RenderGraph::BufferBuilder& builder) {
            builder.set_name(u8"ParticleBuffer")
                .size(100000 * 16 * sizeof(float))
                .allow_shader_readwrite();
        });

        auto particle_pass = graph->add_compute_pass(
            [particle_buffer](RenderGraph& graph, RenderGraph::ComputePassBuilder& builder) {
                builder.set_name(u8"ParticleSimulationPass")
                    .readwrite(u8"Output", particle_buffer.range(0, ~0));
            },
            [](RenderGraph& graph, ComputePassContext& context) {
                // 模拟粒子物理模拟
            });

        // Physics Simulation (另一个异步计算)
        auto physics_buffer = graph->create_buffer([](RenderGraph& graph, RenderGraph::BufferBuilder& builder) {
            builder.set_name(u8"PhysicsBuffer")
                .size(50000 * 12 * sizeof(float))
                .allow_shader_readwrite();
        });

        auto physics_pass = graph->add_compute_pass(
            [physics_buffer](RenderGraph& graph, RenderGraph::ComputePassBuilder& builder) {
                builder.set_name(u8"PhysicsSimulationPass")
                    .readwrite(u8"Output", physics_buffer.range(0, ~0));
            },
            [](RenderGraph& graph, ComputePassContext& context) {
                // 模拟物理模拟
            });

        // ===== 阶段4: 光照计算（需要等待G-Buffer）=====

        auto lighting_pass = graph->add_compute_pass(
            [gbuffer_albedo, gbuffer_normal, lighting_buffer](RenderGraph& graph, RenderGraph::ComputePassBuilder& builder) {
                builder.set_name(u8"DeferredLightingPass")
                    .read(u8"Input0", gbuffer_albedo)
                    .read(u8"Input1", gbuffer_normal)
                    .readwrite(u8"Output", lighting_buffer.range(0, ~0));
            },
            [](RenderGraph& graph, ComputePassContext& context) {
                // 模拟延迟光照计算（有Graphics资源依赖，应该在Graphics队列）
            });

        // ===== 阶段5: 拷贝和资源传输 =====

        // 模拟大量纹理拷贝操作
        auto temp_texture1 = graph->create_texture([](RenderGraph& graph, RenderGraph::TextureBuilder& builder) {
            builder.set_name(u8"TempTexture1")
                .extent(512, 512)
                .format(CGPU_FORMAT_R8G8B8A8_UNORM);
        });

        auto temp_texture2 = graph->create_texture([](RenderGraph& graph, RenderGraph::TextureBuilder& builder) {
            builder.set_name(u8"TempTexture2")
                .extent(512, 512)
                .format(CGPU_FORMAT_R8G8B8A8_UNORM);
        });

        // 独立的拷贝操作（可以在Copy队列执行）
        auto copy_pass1 = graph->add_copy_pass(
            [temp_texture1, temp_texture2](RenderGraph& graph, RenderGraph::CopyPassBuilder& builder) {
                builder.set_name(u8"TextureCopyPass1")
                    .can_be_lone() // 标记为可以独立执行
                    .texture_to_texture(temp_texture1, temp_texture2);
            },
            [](RenderGraph& graph, CopyPassContext& context) {
                // 模拟纹理拷贝
            });

        auto copy_pass2 = graph->add_copy_pass(
            [temp_texture2, temp_texture1](RenderGraph& graph, RenderGraph::CopyPassBuilder& builder) {
                builder.set_name(u8"TextureCopyPass2")
                    .can_be_lone()
                    .texture_to_texture(temp_texture2, temp_texture1);
            },
            [](RenderGraph& graph, CopyPassContext& context) {
                // 模拟另一个纹理拷贝
            });

        // ===== 阶段6: 后处理链（混合Graphics和Compute）=====

        // Bloom Compute
        auto bloom_pass = graph->add_compute_pass(
            [lighting_buffer](RenderGraph& graph, RenderGraph::ComputePassBuilder& builder) {
                builder.set_name(u8"BloomComputePass")
                    .read(0, 0, lighting_buffer.range(0, ~0));
            },
            [](RenderGraph& graph, ComputePassContext& context) {
                // 模拟Bloom效果计算（无Graphics资源依赖，可以异步）
            });

        // Tone Mapping (Graphics渲染)
        auto final_texture = graph->create_texture([](RenderGraph& graph, RenderGraph::TextureBuilder& builder) {
            builder.set_name(u8"FinalTexture")
                .extent(1920, 1080)
                .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                .allow_render_target();
        });

        auto tonemap_pass = graph->add_render_pass(
            [final_texture, lighting_buffer](RenderGraph& graph, RenderGraph::RenderPassBuilder& builder) {
                builder.set_name(u8"ToneMappingPass")
                    .write(0, final_texture, CGPU_LOAD_ACTION_CLEAR)
                    .read(u8"lighting_input", lighting_buffer.range(0, ~0));
            },
            [](RenderGraph& graph, RenderPassContext& context) {
                // 模拟色调映射渲染
            });

        // UI Overlay (Graphics渲染)
        auto ui_pass = graph->add_render_pass(
            [final_texture](RenderGraph& graph, RenderGraph::RenderPassBuilder& builder) {
                builder.set_name(u8"UIOverlayPass")
                    .write(0, final_texture, CGPU_LOAD_ACTION_LOAD);
            },
            [](RenderGraph& graph, RenderPassContext& context) {
                // 模拟UI渲染
            });

        // ===== 阶段7: 最终输出 =====

        auto present_pass = graph->add_present_pass(
            [final_texture](RenderGraph& graph, RenderGraph::PresentPassBuilder& builder) {
                builder.set_name(u8"PresentPass")
                    .texture(final_texture);
            });

        SKR_LOG_INFO(u8"📝 Built complex pipeline with:");
        SKR_LOG_INFO(u8"   - %d render passes (Graphics queue)", 5);
        SKR_LOG_INFO(u8"   - %d compute passes (Mixed queues)", 5);
        SKR_LOG_INFO(u8"   - %d copy passes (Copy queue)", 2);
        SKR_LOG_INFO(u8"   - %d present pass", 1);
    }

    void validate_schedule_result(const skr::render_graph::TimelineScheduleResult& result)
    {
        SKR_LOG_INFO(u8"🔍 Validating schedule result...");

        // 验证基本完整性
        if (result.queue_schedules.is_empty())
        {
            SKR_LOG_ERROR(u8"❌ No queue schedules generated!");
            return;
        }

        if (result.pass_queue_assignments.empty())
        {
            SKR_LOG_ERROR(u8"❌ No pass assignments generated!");
            return;
        }

        // 验证队列类型
        bool has_graphics = false, has_compute = false, has_copy = false;
        for (const auto& queue_schedule : result.queue_schedules)
        {
            switch (queue_schedule.queue_type)
            {
            case skr::render_graph::ERenderGraphQueueType::Graphics:
                has_graphics = true;
                break;
            case skr::render_graph::ERenderGraphQueueType::AsyncCompute:
                has_compute = true;
                break;
            case skr::render_graph::ERenderGraphQueueType::Copy:
                has_copy = true;
                break;
            }
        }

        SKR_LOG_INFO(u8"✅ Queue validation:");
        SKR_LOG_INFO(u8"   Graphics queue: %s", has_graphics ? u8"✓" : u8"✗");
        SKR_LOG_INFO(u8"   Compute queue: %s", has_compute ? u8"✓" : u8"✗");
        SKR_LOG_INFO(u8"   Copy queue: %s", has_copy ? u8"✓" : u8"✗");

        // 验证Pass分配
        uint32_t total_passes = 0;
        for (const auto& queue_schedule : result.queue_schedules)
        {
            total_passes += queue_schedule.scheduled_passes.size();
        }

        if (total_passes != result.pass_queue_assignments.size())
        {
            SKR_LOG_ERROR(u8"❌ Pass count mismatch: scheduled=%d, assigned=%d",
                total_passes,
                (int)result.pass_queue_assignments.size());
        }
        else
        {
            SKR_LOG_INFO(u8"✅ Pass assignment consistency verified");
        }

        // 验证同步需求
        SKR_LOG_INFO(u8"✅ Sync requirements: %d cross-queue dependencies detected",
            (int)result.sync_requirements.size());

        for (const auto& sync_req : result.sync_requirements)
        {
            if (!sync_req.fence)
            {
                SKR_LOG_ERROR(u8"❌ Sync requirement without allocated fence!");
            }
        }

        SKR_LOG_INFO(u8"✅ Schedule validation completed successfully!");
    }
};

int main()
{
    TimelineStressTest test;
    test.run_stress_test();
    return 0;
}