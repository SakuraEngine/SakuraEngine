# SakuraEngine RenderGraph 系统

## 概述

RenderGraph 是 SakuraEngine 的现代化渲染框架，提供声明式的渲染管线构建方式。系统自动处理资源生命周期、内存别名、同步屏障等底层细节，让开发者专注于渲染逻辑。

## 核心概念

### 资源节点 (Resource Node)

资源节点代表 GPU 资源的虚拟句柄：

- **Texture**: 纹理资源
- **Buffer**: 缓冲区资源
- **Imported**: 外部导入的资源

### 渲染通道 (Pass Node)

渲染通道定义 GPU 工作负载：

- **Render Pass**: 图形渲染通道
- **Compute Pass**: 计算着色器通道
- **Copy Pass**: 资源拷贝通道
- **Present Pass**: 呈现通道

### 黑板系统 (Blackboard)

用于在 Pass 之间共享命名资源的全局存储。

## 基本使用

### 创建 RenderGraph

```cpp
#include "SkrRenderGraph/frontend/render_graph.hpp"

// 创建 RenderGraph
auto* graph = RenderGraph::create([](RenderGraph::RenderGraphBuilder& builder) {
    builder.with_device(device)
           .with_gfx_queue(gfx_queue)
           .enable_memory_aliasing();
});

// 销毁
RenderGraph::destroy(graph);
```

### 创建资源

#### 纹理资源

```cpp
// 创建纹理
auto depth_texture = graph.create_texture(
    [](TextureBuilder& builder) {
        builder.set_name(u8"depth_buffer")
               .extent(1920, 1080)
               .format(CGPU_FORMAT_D32_SFLOAT)
               .allow_depth_stencil();
    }
);

// 创建带 Mip 的纹理
auto color_texture = graph.create_texture(
    [](TextureBuilder& builder) {
        builder.set_name(u8"color_rt")
               .extent(1920, 1080)
               .format(CGPU_FORMAT_R8G8B8A8_UNORM)
               .allow_render_target()
               .allow_shader_read()
               .mip_levels(5);
    }
);
```

#### 缓冲区资源

```cpp
// 创建顶点缓冲
auto vertex_buffer = graph.create_buffer(
    [](BufferBuilder& builder) {
        builder.set_name(u8"vertex_buffer")
               .size(sizeof(Vertex) * vertex_count)
               .with_flags(CGPU_BCF_PERSISTENT_MAP_BIT)
               .as_vertex_buffer();
    }
);

// 创建存储缓冲
auto storage_buffer = graph.create_buffer(
    [](BufferBuilder& builder) {
        builder.set_name(u8"storage_buffer")
               .size(sizeof(ParticleData) * max_particles)
               .with_flags(CGPU_BCF_PERSISTENT_MAP_BIT)
               .allow_shader_readwrite();
    }
);
```

### 导入外部资源

```cpp
// 导入交换链纹理
CGPUTextureId backbuffer = swapchain->back_buffers[image_index];
auto imported = graph.import_texture(u8"backbuffer", backbuffer);

// 导入外部缓冲
auto imported_buffer = graph.import_buffer(u8"scene_buffer", scene_buffer);
```

## 渲染通道

### 图形渲染通道

```cpp
auto pass = graph.add_render_pass(
    [&](RenderGraph& g, RenderPassBuilder& builder) {
        builder.set_name(u8"main_pass")
               .set_pipeline(render_pipeline)
               .set_root_signature(root_signature);
        
        // 设置渲染目标
        builder.write(0, graph.get_texture(color_texture, CGPU_RESOURCE_STATE_RENDER_TARGET));
        
        // 设置深度缓冲
        builder.set_depth_stencil(
            graph.get_texture(depth_texture, CGPU_RESOURCE_STATE_DEPTH_WRITE),
            CGPU_LOAD_ACTION_CLEAR,
            CGPU_STORE_ACTION_STORE);
        
        // 绑定资源
        builder.read(u8"scene_texture", 
            graph.get_texture(scene_tex, CGPU_RESOURCE_STATE_SHADER_RESOURCE));
        builder.read(u8"scene_data",
            graph.get_buffer(scene_buffer, CGPU_RESOURCE_STATE_SHADER_RESOURCE));
    },
    [=](RenderGraph& g, RenderPassContext& context) {
        CGPURenderPassEncoderId encoder = context.encoder;
        
        // 设置视口
        CGPUViewport viewport = {0, 0, 1920, 1080, 0.0f, 1.0f};
        cgpu_render_encoder_set_viewport(encoder, viewport.x, viewport.y, 
            viewport.width, viewport.height, viewport.min_depth, viewport.max_depth);
        
        // 绑定描述符集
        cgpu_render_encoder_bind_descriptor_set(encoder, desc_set);
        
        // 绘制调用
        cgpu_render_encoder_draw(encoder, vertex_count, 0);
    }
);
```

### 计算通道

```cpp
auto compute_pass = graph.add_compute_pass(
    [&](RenderGraph& g, ComputePassBuilder& builder) {
        builder.set_name(u8"particle_update");
        
        // 读写粒子数据
        builder.readwrite(u8"particles",
            graph.get_buffer(particle_buffer, CGPU_RESOURCE_STATE_UNORDERED_ACCESS));
        
        // 只读输入
        builder.read(u8"delta_time",
            graph.get_buffer(time_buffer, CGPU_RESOURCE_STATE_SHADER_RESOURCE));
    },
    [=](RenderGraph& g, ComputePassContext& context) {
        CGPUComputePassEncoderId encoder = context.encoder;
        
        cgpu_compute_encoder_bind_pipeline(encoder, compute_pipeline);
        cgpu_compute_encoder_bind_descriptor_set(encoder, compute_desc_set);
        
        // 派发计算
        uint32_t group_count = (particle_count + 63) / 64;
        cgpu_compute_encoder_dispatch(encoder, group_count, 1, 1);
    }
);
```

### 拷贝通道

```cpp
auto copy_pass = graph.add_copy_pass(
    [&](RenderGraph& g, CopyPassBuilder& builder) {
        builder.set_name(u8"mip_gen")
               .texture_to_texture(
                   graph.get_texture(src_texture),
                   graph.get_texture(dst_texture));
    },
    [=](RenderGraph& g, CopyPassContext& context) {
        // 执行拷贝操作
        // 通常框架会自动处理简单拷贝
    }
);
```

## 黑板系统

黑板用于在多个 Pass 之间共享资源：

```cpp
// 添加资源到黑板
graph.get_blackboard().add_texture(u8"g_buffer_albedo", albedo_texture);
graph.get_blackboard().add_buffer(u8"scene_constants", constant_buffer);

// 在其他地方获取
auto albedo = graph.get_blackboard().texture(u8"g_buffer_albedo");
auto constants = graph.get_blackboard().buffer(u8"scene_constants");
```

## 内存别名

RenderGraph 支持自动内存别名以优化内存使用：

```cpp
// 启用内存别名
builder.enable_memory_aliasing();

// 系统会自动分析资源生命周期
// 不重叠的资源会共享同一块物理内存
```

## 多帧资源

对于需要跨帧保持的资源：

```cpp
// 创建历史纹理
auto history_texture = graph.create_texture(
    [](TextureBuilder& builder) {
        builder.set_name(u8"taa_history")
               .extent(1920, 1080)
               .format(CGPU_FORMAT_R16G16B16A16_SFLOAT)
               .allow_render_target()
               .allow_shader_read();
    }
);

// 标记为需要跨帧保持
// 通过黑板系统在帧之间传递
```

## 调试与可视化

### 资源和 Pass 命名

```cpp
// 为调试设置名称
builder.set_name(u8"shadow_map_pass");
texture_builder.set_name(u8"shadow_map");
buffer_builder.set_name(u8"light_data");
```

### 导出可视化

```cpp
#include "SkrRenderGraph/frontend/render_graph_viz.hpp"

// 导出 GraphViz 格式
std::ofstream outf("render_graph.dot");
RenderGraphViz::write_graphviz(graph, outf);
outf.close();

// 使用 graphviz 工具生成图片
// dot -Tpng render_graph.dot -o render_graph.png
```

### 性能分析

```cpp
// 自定义性能分析器
class MyProfiler : public RenderGraphProfiler {
public:
    void on_pass_begin(RenderGraph& graph, 
                      RenderGraphFrameExecutor& executor,
                      PassNode& pass) override {
        // 记录 Pass 开始时间
        SKR_LOG_INFO(u8"Pass %s begin", pass.get_name());
    }
    
    void on_pass_end(RenderGraph& graph,
                    RenderGraphFrameExecutor& executor, 
                    PassNode& pass) override {
        // 记录 Pass 结束时间
        SKR_LOG_INFO(u8"Pass %s end", pass.get_name());
    }
};

// 使用分析器
MyProfiler profiler;
graph.execute(&profiler);
```

## 高级特性

### 条件执行

```cpp
// 根据条件添加 Pass
if (enable_shadows) {
    auto shadow_pass = graph.add_render_pass(
        [&](RenderGraph& g, RenderPassBuilder& builder) {
            builder.set_name(u8"shadow_mapping");
            // ... 配置阴影 Pass
        },
        [](RenderGraph& g, RenderPassContext& context) {
            // ... 渲染阴影贴图
        }
    );
}
```

### 资源屏障优化

RenderGraph 自动插入必要的资源屏障：

```cpp
// 资源状态自动转换
// 从 SHADER_RESOURCE 到 RENDER_TARGET
builder.write(0, graph.get_texture(texture, CGPU_RESOURCE_STATE_RENDER_TARGET));

// 系统会在需要时插入屏障
// 开发者无需手动管理
```

### Present Pass

```cpp
// 添加呈现通道
auto present_pass = graph.add_present_pass(
    [&](RenderGraph& g, PresentPassBuilder& builder) {
        builder.set_name(u8"present")
               .swapchain(swapchain, backbuffer_index)
               .texture(final_texture);
    }
);
```

## 最佳实践

### DO - 推荐做法

1. **合理划分 Pass**
   - 根据渲染阶段划分（深度、不透明、透明等）
   - 考虑并行机会

2. **正确声明资源使用**
   - 精确指定读写权限
   - 使用合适的资源状态

3. **利用黑板系统**
   - 共享跨 Pass 资源
   - 避免重复创建

### DON'T - 避免做法

1. **避免过细的 Pass 划分**
   - 过多小 Pass 增加开销
   - 合并相关渲染操作

2. **不要手动管理屏障**
   - 依赖 RenderGraph 自动管理
   - 避免状态冲突

3. **避免每帧重建图**
   - 缓存不变的部分
   - 只更新动态内容

## 示例：延迟渲染管线

```cpp
// G-Buffer Pass
auto gbuffer_pass = graph.add_render_pass(
    [&](RenderGraph& g, RenderPassBuilder& builder) {
        builder.set_name(u8"gbuffer_pass");
        
        // 写入 G-Buffer
        builder.write(0, g.get_texture(albedo_rt, CGPU_RESOURCE_STATE_RENDER_TARGET));
        builder.write(1, g.get_texture(normal_rt, CGPU_RESOURCE_STATE_RENDER_TARGET));
        builder.write(2, g.get_texture(motion_rt, CGPU_RESOURCE_STATE_RENDER_TARGET));
        builder.set_depth_stencil(g.get_texture(depth_buffer, CGPU_RESOURCE_STATE_DEPTH_WRITE));
        
        // 添加到黑板供后续使用
        g.get_blackboard().add_texture(u8"gbuffer_albedo", albedo_rt);
        g.get_blackboard().add_texture(u8"gbuffer_normal", normal_rt);
        g.get_blackboard().add_texture(u8"gbuffer_depth", depth_buffer);
    },
    [](RenderGraph& g, RenderPassContext& context) {
        // 渲染场景到 G-Buffer
    }
);

// Lighting Pass
auto lighting_pass = graph.add_render_pass(
    [&](RenderGraph& g, RenderPassBuilder& builder) {
        builder.set_name(u8"lighting_pass");
        
        // 读取 G-Buffer
        auto albedo = g.get_blackboard().texture(u8"gbuffer_albedo");
        auto normal = g.get_blackboard().texture(u8"gbuffer_normal");
        auto depth = g.get_blackboard().texture(u8"gbuffer_depth");
        
        builder.read(u8"albedo", g.get_texture(albedo, CGPU_RESOURCE_STATE_SHADER_RESOURCE));
        builder.read(u8"normal", g.get_texture(normal, CGPU_RESOURCE_STATE_SHADER_RESOURCE));
        builder.read(u8"depth", g.get_texture(depth, CGPU_RESOURCE_STATE_SHADER_RESOURCE));
        
        // 输出到最终颜色
        builder.write(0, g.get_texture(final_color, CGPU_RESOURCE_STATE_RENDER_TARGET));
    },
    [](RenderGraph& g, RenderPassContext& context) {
        // 执行光照计算
    }
);
```

## 总结

RenderGraph 提供了强大而灵活的渲染管线抽象。通过声明式 API、自动资源管理和优化，开发者可以专注于渲染算法而不是底层细节。合理使用 RenderGraph 可以获得更好的性能和更清晰的代码结构。