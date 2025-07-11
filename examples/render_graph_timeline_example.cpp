// Timeline多队列渲染示例
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/api/timeline_builder.hpp"
#include "SkrGraphics/api.h"

using namespace skr;
using namespace skr::render_graph;

// 创建一个使用Timeline的渲染图示例
void create_timeline_render_graph_example(CGPUDeviceId device, CGPUQueueId gfx_queue)
{
    // 1. 创建RenderGraph
    auto* graph = RenderGraph::create([=](RenderGraph::RenderGraphBuilder& builder) {
        builder.with_device(device)
               .with_gfx_queue(gfx_queue)
               .enable_memory_aliasing();
    });
    
    // 2. 配置并添加Timeline Phase
    auto timeline_builder = RenderGraphTimelineExtension::create_timeline_builder(graph);
    auto* timeline_phase = timeline_builder
        .enable_async_compute(true)      // 启用异步计算
        .enable_copy_queue(true)         // 启用拷贝队列
        .enable_load_balancing(true)     // 启用负载均衡
        .set_load_balance_threshold(1.3f) // 设置负载均衡阈值
        .build();
    
    // 3. 创建资源
    auto depth_buffer = graph->create_texture([](RenderGraph& g, TextureBuilder& builder) {
        builder.set_name(u8"DepthBuffer")
               .extent(1920, 1080)
               .format(CGPU_FORMAT_D32_SFLOAT)
               .allow_depth_stencil();
    });
    
    auto shadow_map = graph->create_texture([](RenderGraph& g, TextureBuilder& builder) {
        builder.set_name(u8"ShadowMap")
               .extent(2048, 2048)
               .format(CGPU_FORMAT_D16_UNORM)
               .allow_depth_stencil();
    });
    
    auto ssao_buffer = graph->create_texture([](RenderGraph& g, TextureBuilder& builder) {
        builder.set_name(u8"SSAO")
               .extent(1920, 1080)
               .format(CGPU_FORMAT_R8_UNORM)
               .allow_render_target();
    });
    
    auto compute_buffer = graph->create_buffer([](RenderGraph& g, BufferBuilder& builder) {
        builder.set_name(u8"ComputeBuffer")
               .size(1024 * 1024 * 16) // 16MB
               .allow_shader_readwrite()
               .as_storage_buffer();
    });
    
    // 4. 创建Shadow Pass (Graphics队列)
    auto shadow_pass = graph->add_render_pass(
        [=](RenderGraph& g, RenderPassBuilder& builder) {
            builder.set_name(u8"ShadowPass")
                   .set_depth_stencil(shadow_map->get_dsv({}));
        },
        [](RenderGraph& g, RenderPassContext& ctx) {
            // 渲染阴影贴图
            auto* encoder = ctx.encoder;
            // ... 绘制命令
        }
    );
    
    // 5. 创建异步计算Pass (AsyncCompute队列)
    auto compute_pass = graph->add_compute_pass(
        [=](RenderGraph& g, ComputePassBuilder& builder) {
            builder.set_name(u8"ParticleSimulation")
                   .readwrite(0, 0, compute_buffer);
        },
        [](RenderGraph& g, ComputePassContext& ctx) {
            // 粒子模拟计算
            auto* encoder = ctx.encoder;
            cgpu_compute_encoder_dispatch(encoder, 256, 1, 1);
        }
    );
    
    // 标记为异步计算
    RenderGraphTimelineExtension::mark_async_compute(graph, compute_pass);
    
    // 6. 创建SSAO Pass (可能在AsyncCompute队列)
    auto ssao_pass = graph->add_compute_pass(
        [=](RenderGraph& g, ComputePassBuilder& builder) {
            builder.set_name(u8"SSAO_Compute")
                   .read(0, 0, depth_buffer->get_srv({}))
                   .readwrite(0, 1, ssao_buffer->get_uav({}));
        },
        [](RenderGraph& g, ComputePassContext& ctx) {
            // SSAO计算
            auto* encoder = ctx.encoder;
            cgpu_compute_encoder_dispatch(encoder, 60, 34, 1); // 1920/32, 1080/32
        }
    );
    
    // 标记为异步计算（如果没有紧密的图形依赖）
    RenderGraphTimelineExtension::mark_async_compute(graph, ssao_pass);
    
    // 7. 创建主渲染Pass (Graphics队列)
    auto main_pass = graph->add_render_pass(
        [=](RenderGraph& g, RenderPassBuilder& builder) {
            builder.set_name(u8"MainPass")
                   .write(0, render_target)
                   .set_depth_stencil(depth_buffer->get_dsv({}))
                   .read(u8"shadowMap", shadow_map->get_srv({}))
                   .read(u8"ssao", ssao_buffer->get_srv({}))
                   .read(u8"particleData", compute_buffer);
        },
        [](RenderGraph& g, RenderPassContext& ctx) {
            // 主渲染
            auto* encoder = ctx.encoder;
            // ... 绘制命令
        }
    );
    
    // 8. 创建大型纹理拷贝 (Copy队列)
    auto large_texture = graph->create_texture([](RenderGraph& g, TextureBuilder& builder) {
        builder.set_name(u8"LargeTexture")
               .extent(4096, 4096)
               .format(CGPU_FORMAT_R8G8B8A8_UNORM)
               .allow_render_target();
    });
    
    auto copy_pass = graph->add_copy_pass(
        [=](RenderGraph& g, CopyPassBuilder& builder) {
            builder.set_name(u8"LargeTextureCopy")
                   .can_be_lone() // 标记可以独立执行
                   .texture_to_texture(
                       large_texture->get_subresource(0, 0),
                       backup_texture->get_subresource(0, 0)
                   );
        },
        [](RenderGraph& g, CopyPassContext& ctx) {
            // 拷贝已经由builder配置
        }
    );
    
    // 9. 编译和执行
    graph->compile();
    
    // 查看Timeline信息
    auto& timelines = timeline_phase->get_timelines();
    for (const auto& timeline : timelines) {
        SKR_LOG_INFO(u8"Queue %s: %d passes, workload: %llu",
                     get_queue_type_name(timeline.type),
                     (int)timeline.passes.size(),
                     timeline.estimated_workload);
    }
    
    // 执行渲染图
    graph->execute();
    
    // 10. 清理
    RenderGraph::destroy(graph);
}

// 更复杂的示例：多GPU Timeline
void create_multi_gpu_timeline_example(CGPUDeviceId device1, CGPUDeviceId device2)
{
    // TODO: 未来扩展 - 支持多GPU Timeline
    // 这需要扩展TimelinePhase以支持多设备调度
}

// 动态调整示例
class AdaptiveTimelineExample
{
public:
    void update(RenderGraph* graph, TimelinePhase* timeline_phase)
    {
        // 获取运行时统计
        auto& timelines = timeline_phase->get_timelines();
        
        // 分析队列利用率
        float total_utilization = 0.0f;
        for (const auto& timeline : timelines) {
            total_utilization += timeline.utilization;
        }
        
        // 动态调整策略
        if (total_utilization < 0.5f) {
            // 利用率低，可以更激进地使用异步计算
            // 通过重新配置Phase或调整Pass提示
        } else if (total_utilization > 0.9f) {
            // 利用率高，可能需要减少跨队列同步
        }
    }
};

// 性能分析示例
class TimelineProfiler : public RenderGraphProfiler
{
public:
    void on_pass_begin(RenderGraph& graph, RenderGraphFrameExecutor& executor, PassNode& pass) override
    {
        // 记录Pass开始时间和所在队列
        auto queue_idx = pass.get_user_data("assigned_queue");
        pass_start_times_[&pass] = get_current_time();
        pass_queues_[&pass] = queue_idx;
    }
    
    void on_pass_end(RenderGraph& graph, RenderGraphFrameExecutor& executor, PassNode& pass) override
    {
        // 计算Pass执行时间
        auto duration = get_current_time() - pass_start_times_[&pass];
        auto queue_idx = pass_queues_[&pass];
        
        // 更新队列统计
        queue_stats_[queue_idx].total_time += duration;
        queue_stats_[queue_idx].pass_count++;
    }
    
    void report()
    {
        for (auto& [queue_idx, stats] : queue_stats_) {
            float avg_time = stats.total_time / stats.pass_count;
            SKR_LOG_INFO(u8"Queue %d: avg pass time: %.2fms", queue_idx, avg_time);
        }
    }
    
private:
    struct QueueStats {
        float total_time = 0.0f;
        uint32_t pass_count = 0;
    };
    
    skr::HashMap<PassNode*, float> pass_start_times_;
    skr::HashMap<PassNode*, uint32_t> pass_queues_;
    skr::HashMap<uint32_t, QueueStats> queue_stats_;
    
    float get_current_time() { 
        // 实现时间获取
        return 0.0f; 
    }
};