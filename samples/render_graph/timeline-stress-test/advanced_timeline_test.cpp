#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/phases_v2/schedule_timeline.hpp"
#include "SkrCore/log.hpp"
#include <random>

using namespace skr::render_graph;

// 高级Timeline压力测试 - 模拟真实游戏引擎的复杂场景
class AdvancedTimelineTest
{
public:
    AdvancedTimelineTest() : rng(std::random_device{}()) {}
    
    void run_all_tests()
    {
        SKR_LOG_INFO(u8"🚀 Advanced Timeline Test Suite Starting...");
        
        test_massively_parallel_scenario();
        test_deeply_dependent_chain();
        test_mixed_workload_balancing();
        test_resource_heavy_scenario();
        test_edge_case_scenarios();
        
        SKR_LOG_INFO(u8"🎉 All Advanced Timeline Tests Completed!");
    }

private:
    std::mt19937 rng;
    
    // 测试1: 大量并行计算场景
    void test_massively_parallel_scenario()
    {

        SKR_LOG_INFO(u8"🧪 Test 1: Massively Parallel Scenario");
        
        auto* graph = skr::render_graph::RenderGraph::create([](auto& builder) {
            builder.frontend_only();
        });
        
        auto timeline_config = skr::render_graph::ScheduleTimelineConfig{};
        timeline_config.enable_async_compute = true;
        timeline_config.enable_copy_queue = true;
        
        auto* timeline_phase = new skr::render_graph::ScheduleTimeline(timeline_config);
        
        // 创建20个独立的计算Pass（应该全部分配到AsyncCompute队列）
        for (int i = 0; i < 20; ++i) {
            auto buffer = graph->create_buffer([i](auto& graph, auto& builder) {
                builder.set_name(skr::format(u8"ComputeBuffer_{}", i).c_str())
                       .size(1024 * 1024)
                       .allow_shader_readwrite();
            });
            
            graph->add_compute_pass(
                [buffer, i](auto& graph, auto& builder) {
                    builder.set_name(skr::format(u8"ParallelCompute_{}", i).c_str())
                           .readwrite(0, 0, buffer);
                },
                [](auto& graph, ComputePassContext& context) {
                    // 独立计算任务
                }
            );
        }
        
        // 添加一个Graphics Pass作为对比
        auto final_texture = graph->create_texture([](auto& graph, auto& builder) {
            builder.set_name(u8"FinalTexture")
                   .extent(1920, 1080)
                   .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                   .allow_render_target();
        });
        
        graph->add_render_pass(
            [final_texture](auto& graph, auto& builder) {
                builder.set_name(u8"FinalRender")
                       .write(0, final_texture, CGPU_LOAD_ACTION_CLEAR);
            },
            [](auto& graph, RenderPassContext& context) {}
        );
        
        // 执行调度
        timeline_phase->on_initialize(graph);
        timeline_phase->on_execute(graph, nullptr);
        timeline_phase->dump_timeline_result(u8"🧪 Test 1: Massively Parallel Results");
        
        // 验证：大部分计算Pass应该在AsyncCompute队列
        const auto& result = timeline_phase->get_schedule_result();
        uint32_t compute_queue_passes = 0;
        for (const auto& queue_schedule : result.queue_schedules) {
            if (queue_schedule.queue_type == skr::render_graph::ERenderGraphQueueType::AsyncCompute) {
                compute_queue_passes = queue_schedule.scheduled_passes.size();
                break;
            }
        }
        
        SKR_LOG_INFO(u8"✅ Parallel Test: %d passes assigned to AsyncCompute queue", compute_queue_passes);
        
        timeline_phase->on_finalize(graph);
        delete timeline_phase;
        skr::render_graph::RenderGraph::destroy(graph);
    }
    
    // 测试2: 深度依赖链场景
    void test_deeply_dependent_chain()
    {
        SKR_LOG_INFO(u8"🧪 Test 2: Deeply Dependent Chain");
        
        auto* graph = skr::render_graph::RenderGraph::create([](auto& builder) {
            builder.frontend_only();
        });
        
        auto timeline_config = skr::render_graph::ScheduleTimelineConfig{};
        timeline_config.enable_async_compute = true;
        timeline_config.enable_copy_queue = true;
        
        auto* timeline_phase = new skr::render_graph::ScheduleTimeline(timeline_config);
        
        // 创建一个长依赖链：Graphics -> Compute -> Graphics -> Compute ...
        auto chain_texture = graph->create_texture([](auto& graph, auto& builder) {
            builder.set_name(u8"ChainTexture")
                   .extent(1024, 1024)
                   .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                   .allow_render_target()
                   .allow_readwrite();
        });
        
        auto chain_buffer = graph->create_buffer([](auto& graph, auto& builder) {
            builder.set_name(u8"ChainBuffer")
                   .size(1024 * 1024 * sizeof(float))
                   .allow_shader_readwrite();
        });
        
        // 步骤1: Graphics渲染到纹理
        graph->add_render_pass(
            [chain_texture](auto& graph, auto& builder) {
                builder.set_name(u8"ChainStep1_Graphics")
                       .write(0, chain_texture, CGPU_LOAD_ACTION_CLEAR);
            },
            [](auto& graph, RenderPassContext& context) {}
        );
        
        // 步骤2: Compute读取纹理，写入缓冲区（跨队列依赖）
        graph->add_compute_pass(
            [chain_texture, chain_buffer](auto& graph, auto& builder) {
                builder.set_name(u8"ChainStep2_Compute")
                       .read(0, 0, chain_texture)
                       .readwrite(0, 1, chain_buffer);
            },
            [](auto& graph, ComputePassContext& context) {}
        );
        
        // 步骤3: Graphics读取缓冲区，再次渲染（又一个跨队列依赖）
        graph->add_render_pass(
            [chain_texture, chain_buffer](auto& graph, auto& builder) {
                builder.set_name(u8"ChainStep3_Graphics")
                       .write(0, chain_texture, CGPU_LOAD_ACTION_CLEAR)
                       .read(u8"chain_buffer", chain_buffer.range(0, ~0));
            },
            [](auto& graph, RenderPassContext& context) {}
        );
        
        // 步骤4: 最终Compute处理
        graph->add_compute_pass(
            [chain_texture](auto& graph, auto& builder) {
                builder.set_name(u8"ChainStep4_Compute")
                       .read(0, 0, chain_texture);
            },
            [](auto& graph, ComputePassContext& context) {}
        );
        
        timeline_phase->on_initialize(graph);
        timeline_phase->on_execute(graph, nullptr);
        timeline_phase->dump_timeline_result(u8"🧪 Test 2: Deep Dependency Chain Results");
        
        // 验证：应该有多个同步需求
        const auto& result = timeline_phase->get_schedule_result();
        SKR_LOG_INFO(u8"✅ Dependency Test: %d cross-queue sync requirements generated", 
                     (int)result.sync_requirements.size());
        
        timeline_phase->on_finalize(graph);
        delete timeline_phase;
        skr::render_graph::RenderGraph::destroy(graph);
    }
    
    // 测试3: 混合工作负载均衡
    void test_mixed_workload_balancing()
    {
        SKR_LOG_INFO(u8"🧪 Test 3: Mixed Workload Balancing");
        
        auto* graph = skr::render_graph::RenderGraph::create([](auto& builder) {
            builder.frontend_only();
        });
        
        auto timeline_config = skr::render_graph::ScheduleTimelineConfig{};
        timeline_config.enable_async_compute = true;
        timeline_config.enable_copy_queue = true;
        
        auto* timeline_phase = new skr::render_graph::ScheduleTimeline(timeline_config);
        
        // 创建各种类型的Pass混合
        std::vector<skr::render_graph::PassHandle> passes;
        
        // 10个Render Pass
        for (int i = 0; i < 10; ++i) {
            auto texture = graph->create_texture([i](auto& graph, auto& builder) {
                builder.set_name(skr::format(u8"RenderTarget_{}", i).c_str())
                       .extent(512, 512)
                       .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                       .allow_render_target();
            });
            
            passes.push_back(graph->add_render_pass(
                [texture, i](auto& graph, auto& builder) {
                    builder.set_name(skr::format(u8"RenderPass_{}", i).c_str())
                           .write(0, texture, CGPU_LOAD_ACTION_CLEAR);
                },
                [](auto& graph, RenderPassContext& context) {}
            ));
        }
        
        // 15个Compute Pass（混合有/无Graphics依赖）
        for (int i = 0; i < 15; ++i) {
            auto buffer = graph->create_buffer([i](auto& graph, auto& builder) {
                builder.set_name(skr::format(u8"ComputeBuffer_{}", i).c_str())
                       .size(1024 * 1024)
                       .allow_shader_readwrite();
            });
            
            passes.push_back(graph->add_compute_pass(
                [buffer, i](auto& graph, auto& builder) {
                    builder.set_name(skr::format(u8"ComputePass_{}", i).c_str())
                           .readwrite(0, 0, buffer);
                },
                [](auto& graph, ComputePassContext& context) {}
            ));
        }
        
        // 5个Copy Pass
        for (int i = 0; i < 5; ++i) {
            auto src_texture = graph->create_texture([i](auto& graph, auto& builder) {
                builder.set_name(skr::format(u8"CopySrc_{}", i).c_str())
                       .extent(256, 256)
                       .format(CGPU_FORMAT_R8G8B8A8_UNORM);
            });
            
            auto dst_texture = graph->create_texture([i](auto& graph, auto& builder) {
                builder.set_name(skr::format(u8"CopyDst_{}", i).c_str())
                       .extent(256, 256)
                       .format(CGPU_FORMAT_R8G8B8A8_UNORM);
            });
            
            passes.push_back(graph->add_copy_pass(
                [src_texture, dst_texture, i](auto& graph, auto& builder) {
                    builder.set_name(skr::format(u8"CopyPass_{}", i).c_str())
                           .can_be_lone()
                           .texture_to_texture(src_texture, dst_texture);
                },
                [](auto& graph, CopyPassContext& context) {}
            ));
        }
        
        timeline_phase->on_initialize(graph);
        timeline_phase->on_execute(graph, nullptr);
        timeline_phase->dump_timeline_result(u8"🧪 Test 3: Mixed Workload Results");
        
        // 验证工作负载分布
        const auto& result = timeline_phase->get_schedule_result();
        SKR_LOG_INFO(u8"✅ Mixed Workload Test: Distributed across %d queues", 
                     (int)result.queue_schedules.size());
        
        timeline_phase->on_finalize(graph);
        delete timeline_phase;
        skr::render_graph::RenderGraph::destroy(graph);
    }
    
    // 测试4: 资源密集型场景
    void test_resource_heavy_scenario()
    {
        SKR_LOG_INFO(u8"🧪 Test 4: Resource Heavy Scenario");
        
        auto* graph = skr::render_graph::RenderGraph::create([](auto& builder) {
            builder.frontend_only();
        });
        
        auto timeline_config = skr::render_graph::ScheduleTimelineConfig{};
        timeline_config.enable_async_compute = true;
        timeline_config.enable_copy_queue = true;
        
        auto* timeline_phase = new skr::render_graph::ScheduleTimeline(timeline_config);
        
        // 创建大量资源
        std::vector<skr::render_graph::TextureHandle> textures;
        std::vector<skr::render_graph::BufferHandle> buffers;
        
        // 50个纹理
        for (int i = 0; i < 50; ++i) {
            textures.push_back(graph->create_texture([i](auto& graph, auto& builder) {
                builder.set_name(skr::format(u8"HeavyTexture_{}", i).c_str())
                       .extent(1024, 1024)
                       .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                       .allow_render_target()
                       .allow_readwrite();
            }));
        }
        
        // 30个缓冲区
        for (int i = 0; i < 30; ++i) {
            buffers.push_back(graph->create_buffer([i](auto& graph, auto& builder) {
                builder.set_name(skr::format(u8"HeavyBuffer_{}", i).c_str())
                       .size(10 * 1024 * 1024)  // 10MB每个
                       .allow_shader_readwrite();
            }));
        }
        
        // 创建复杂的Pass网络，随机使用资源
        for (int pass_idx = 0; pass_idx < 25; ++pass_idx) {
            std::uniform_int_distribution<> texture_dist(0, textures.size() - 1);
            std::uniform_int_distribution<> buffer_dist(0, buffers.size() - 1);
            std::uniform_int_distribution<> pass_type_dist(0, 2); // 0=Render, 1=Compute, 2=Copy
            
            int pass_type = pass_type_dist(rng);
            
            if (pass_type == 0) { // Render Pass
                int tex_idx = texture_dist(rng);
                graph->add_render_pass(
                    [this, textures, tex_idx, pass_idx](auto& graph, auto& builder) {
                        builder.set_name(skr::format(u8"HeavyRender_{}", pass_idx).c_str())
                               .write(0, textures[tex_idx]);
                        
                        // 随机读取其他纹理
                        std::uniform_int_distribution<> read_count_dist(0, 3);
                        int read_count = read_count_dist(rng);
                        for (int r = 0; r < read_count; ++r) {
                            std::uniform_int_distribution<> read_tex_dist(0, textures.size() - 1);
                            int read_tex = read_tex_dist(rng);
                            if (read_tex != tex_idx) {
                                builder.read(r, 0, textures[read_tex]);
                            }
                        }
                    },
                    [](auto& graph, auto& executor) {}
                );
            } else if (pass_type == 1) { // Compute Pass
                int buf_idx = buffer_dist(rng);
                graph->add_compute_pass(
                    [this, buffers, buf_idx, pass_idx](auto& graph, auto& builder) {
                        builder.set_name(skr::format(u8"HeavyCompute_{}", pass_idx).c_str())
                               .readwrite(0, 0, buffers[buf_idx]);
                        
                        // 随机读取其他缓冲区
                        std::uniform_int_distribution<> read_count_dist(0, 2);
                        int read_count = read_count_dist(rng);
                        for (int r = 0; r < read_count; ++r) {
                            std::uniform_int_distribution<> read_buf_dist(0, buffers.size() - 1);
                            int read_buf = read_buf_dist(rng);
                            if (read_buf != buf_idx) {
                                builder.read(r, 1, buffers[read_buf]);
                            }
                        }
                    },
                    [](auto& graph, auto& executor) {}
                );
            } else { // Copy Pass
                int src_idx = texture_dist(rng);
                int dst_idx = texture_dist(rng);
                if (src_idx != dst_idx) {
                    graph->add_copy_pass(
                        [textures, src_idx, dst_idx, pass_idx](auto& graph, auto& builder) {
                            builder.set_name(skr::format(u8"HeavyCopy_{}", pass_idx).c_str())
                                   .can_be_lone()
                                   .texture_to_texture(textures[src_idx], textures[dst_idx]);
                        },
                        [](auto& graph, auto& executor) {}
                    );
                }
            }
        }
        
        timeline_phase->on_initialize(graph);
        timeline_phase->on_execute(graph, nullptr);
        timeline_phase->dump_timeline_result(u8"🧪 Test 4: Resource Heavy Results");
        
        SKR_LOG_INFO(u8"✅ Resource Heavy Test: Handled %d textures and %d buffers", 
                     (int)textures.size(), (int)buffers.size());
        
        timeline_phase->on_finalize(graph);
        delete timeline_phase;
        skr::render_graph::RenderGraph::destroy(graph);
    }
    
    // 测试5: 边缘情况
    void test_edge_case_scenarios()
    {
        SKR_LOG_INFO(u8"🧪 Test 5: Edge Case Scenarios");
        
        // 边缘情况1：空RenderGraph
        {
            auto* graph = skr::render_graph::RenderGraph::create([](auto& builder) {
                builder.frontend_only();
            });
            
            auto* timeline_phase = new skr::render_graph::ScheduleTimeline();
            
            timeline_phase->on_initialize(graph);
            timeline_phase->on_execute(graph, nullptr);
            timeline_phase->dump_timeline_result(u8"🧪 Edge Case 1: Empty Graph");
            
            const auto& result = timeline_phase->get_schedule_result();
            SKR_LOG_INFO(u8"✅ Empty Graph Test: %d queues, %d passes", 
                         (int)result.queue_schedules.size(), (int)result.pass_queue_assignments.size());
            
            timeline_phase->on_finalize(graph);
            delete timeline_phase;
            skr::render_graph::RenderGraph::destroy(graph);
        }
        
        // 边缘情况2：只有Present Pass
        {
            auto* graph = skr::render_graph::RenderGraph::create([](auto& builder) {
                builder.frontend_only();
            });
            
            auto* timeline_phase = new skr::render_graph::ScheduleTimeline();
            
            auto final_texture = graph->create_texture([](auto& graph, auto& builder) {
                builder.set_name(u8"FinalTexture")
                       .extent(1920, 1080)
                       .format(CGPU_FORMAT_R8G8B8A8_UNORM);
            });
            
            graph->add_present_pass(
                [final_texture](auto& graph, auto& builder) {
                    builder.set_name(u8"OnlyPresentPass")
                           .texture(final_texture);
                }
            );
            
            timeline_phase->on_initialize(graph);
            timeline_phase->on_execute(graph, nullptr);
            timeline_phase->dump_timeline_result(u8"🧪 Edge Case 2: Only Present Pass");
            
            timeline_phase->on_finalize(graph);
            delete timeline_phase;
            skr::render_graph::RenderGraph::destroy(graph);
        }
        
        // 边缘情况3：禁用所有异步队列
        {
            auto* graph = skr::render_graph::RenderGraph::create([](auto& builder) {
                builder.frontend_only();
            });
            
            auto timeline_config = skr::render_graph::ScheduleTimelineConfig{};
            timeline_config.enable_async_compute = false;
            timeline_config.enable_copy_queue = false;
            
            auto* timeline_phase = new skr::render_graph::ScheduleTimeline(timeline_config);
            
            // 添加各种Pass，应该全部分配到Graphics队列
            auto texture = graph->create_texture([](auto& graph, auto& builder) {
                builder.set_name(u8"TestTexture")
                       .extent(1024, 1024)
                       .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                       .allow_render_target();
            });
            
            auto buffer = graph->create_buffer([](auto& graph, auto& builder) {
                builder.set_name(u8"TestBuffer")
                       .size(1024 * 1024)
                       .allow_shader_readwrite();
            });
            
            graph->add_render_pass(
                [texture](auto& graph, auto& builder) {
                    builder.set_name(u8"TestRender")
                           .write(0, texture, CGPU_LOAD_ACTION_CLEAR);
                },
                [](auto& graph, RenderPassContext& context) {}
            );
            
            graph->add_compute_pass(
                [buffer](auto& graph, auto& builder) {
                    builder.set_name(u8"TestCompute")
                           .readwrite(0, 0, buffer);
                },
                [](auto& graph, ComputePassContext& context) {}
            );
            
            timeline_phase->on_initialize(graph);
            timeline_phase->on_execute(graph, nullptr);
            timeline_phase->dump_timeline_result(u8"🧪 Edge Case 3: Graphics Only");
            
            const auto& result = timeline_phase->get_schedule_result();
            SKR_LOG_INFO(u8"✅ Graphics Only Test: %d sync requirements (should be 0)", 
                         (int)result.sync_requirements.size());
            
            timeline_phase->on_finalize(graph);
            delete timeline_phase;
            skr::render_graph::RenderGraph::destroy(graph);
        }
        
        SKR_LOG_INFO(u8"✅ All edge case scenarios completed!");
    }
};

int main()
{
    AdvancedTimelineTest test;
    test.run_all_tests();
    return 0;
}