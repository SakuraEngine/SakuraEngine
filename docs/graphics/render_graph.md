# SakuraEngine RenderGraph 系统

## 概述

RenderGraph 是 SakuraEngine 的现代化渲染框架，提供了声明式的渲染管线构建方式。系统自动管理资源生命周期、状态转换和内存优化，让开发者专注于渲染逻辑而非底层资源管理。

## 核心概念

### 架构设计

RenderGraph 采用前后端分离架构：

```
┌─────────────────────────────────────────────────┐
│                   Frontend                       │
│  (Graph Building, Resource Declaration, Pass)    │
├─────────────────────────────────────────────────┤
│                   Backend                        │
│  (Execution, Resource Allocation, Barriers)      │
└─────────────────────────────────────────────────┘
```

- **前端**：提供用户友好的 API，负责图的构建和高层抽象
- **后端**：处理实际执行、资源分配、屏障管理等底层细节

### 基础组件

#### 节点 (Node)

RenderGraph 中的基本单元：

```cpp
// Pass 节点类型
enum class EPassType : uint32_t {
    None = 0,
    Render = 1,    // 渲染 Pass（光栅化）
    Compute = 2,   // 计算 Pass
    Copy = 3,      // 拷贝 Pass
    Present = 4    // 呈现 Pass
};

// 资源节点类型
enum class ResourceType {
    Texture,   // 纹理资源
    Buffer     // 缓冲区资源
};
```

#### 边 (Edge)

表示节点间的依赖关系：

```cpp
// 纹理访问边
- TextureReadEdge      // 着色器资源视图 (SRV)
- TextureRenderEdge    // 渲染目标 (RTV) 或深度模板 (DSV)
- TextureReadWriteEdge // 无序访问视图 (UAV)

// 缓冲区访问边
- BufferReadEdge       // 常量缓冲区 (CBV) 或结构化缓冲区 (SRV)
- BufferReadWriteEdge  // 无序访问缓冲区 (UAV)
- PipelineBufferEdge   // 顶点缓冲区 (VB) 或索引缓冲区 (IB)
```

#### 句柄 (Handle)

类型安全的资源引用：

```cpp
// 资源句柄
using TextureHandle = TypedHandle<Texture, 8, 8, 16>;
using BufferHandle = TypedHandle<Buffer, 8, 8, 16>;

// 句柄包含：
// - 8 位索引
// - 8 位版本
// - 16 位子资源信息（mip level, array slice 等）
```

## 基本使用

### 创建 RenderGraph

```cpp
// 创建 RenderGraph 实例
auto graph = RenderGraph::create();

// 配置选项
RenderGraphBuilder builder;
builder.with_device(device)                    // 设置 GPU 设备
       .with_gfx_queue(graphics_queue)         // 图形队列
       .with_cpy_queue(copy_queue)             // 拷贝队列（可选）
       .enable_memory_aliasing(true)           // 启用内存别名优化
       .set_max_in_flight_frames(3);           // 最大并行帧数

graph->initialize(builder);
```

### 资源声明

#### 创建纹理

```cpp
// 创建内部纹理
auto depth_buffer = graph->create_texture(
    [=](RenderGraph& g, TextureBuilder& builder) {
        builder.set_name("depth_buffer")
               .extent(1920, 1080)
               .format(CGPU_FORMAT_D32_SFLOAT)
               .allow_depth_stencil()
               .sample_count(CGPU_SAMPLE_COUNT_1)
               .usage_flags(CGPU_TCF_USAGE_CAN_SIMULTANEOUS_ACCESS);
    });

// 导入外部纹理
auto swapchain_texture = graph->create_texture(
    [=](RenderGraph& g, TextureBuilder& builder) {
        builder.set_name("backbuffer")
               .import(swapchain->back_buffers[index], 
                      CGPU_RESOURCE_STATE_PRESENT)
               .allow_render_target();
    });

// 创建纹理数组
auto texture_array = graph->create_texture(
    [=](RenderGraph& g, TextureBuilder& builder) {
        builder.set_name("shadow_maps")
               .extent(1024, 1024)
               .array_size(4)  // 4 层阴影贴图
               .format(CGPU_FORMAT_D16_UNORM)
               .allow_depth_stencil();
    });
```

#### 创建缓冲区

```cpp
// 创建常量缓冲区
auto uniform_buffer = graph->create_buffer(
    [=](RenderGraph& g, BufferBuilder& builder) {
        builder.set_name("frame_uniforms")
               .size(sizeof(FrameUniforms))
               .with_flags(CGPU_BCF_UNIFORM)
               .memory_usage(CGPU_MEM_USAGE_GPU_ONLY)
               .as_uniform();
    });

// 创建结构化缓冲区
auto instance_buffer = graph->create_buffer(
    [=](RenderGraph& g, BufferBuilder& builder) {
        builder.set_name("instances")
               .size(sizeof(InstanceData) * instance_count)
               .structured(sizeof(InstanceData))
               .with_flags(CGPU_BCF_USAGE_UAV)
               .as_storage();
    });

// 导入外部缓冲区
auto vertex_buffer = graph->create_buffer(
    [=](RenderGraph& g, BufferBuilder& builder) {
        builder.set_name("vertices")
               .import(vb_resource, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
               .as_vertex_buffer();
    });
```

### Pass 定义

#### 渲染 Pass

```cpp
auto main_pass = graph->add_render_pass(
    // Setup 函数：配置 Pass
    [=](RenderGraph& g, RenderPassBuilder& builder) {
        builder.set_name("main_pass")
               
               // 设置渲染管线
               .set_pipeline(render_pipeline)
               
               // 读取资源
               .read("diffuse_map", diffuse_texture)
               .read("normal_map", normal_texture)
               .sample("shadow_map", shadow_map)
               
               // 写入渲染目标
               .write(0, color_target, 
                     CGPU_LOAD_ACTION_CLEAR, 
                     CGPU_STORE_ACTION_STORE,
                     {0.1f, 0.2f, 0.3f, 1.0f})  // 清除颜色
               
               // 设置深度缓冲
               .set_depth_stencil(
                     depth_buffer,
                     CGPU_LOAD_ACTION_CLEAR,
                     CGPU_STORE_ACTION_STORE,
                     1.0f,  // 清除深度
                     CGPU_LOAD_ACTION_DONT_CARE,
                     CGPU_STORE_ACTION_DONT_CARE,
                     0);    // 清除模板
    },
    
    // Execute 函数：记录渲染命令
    [=](RenderGraph& g, RenderPassContext& context) {
        auto encoder = context.encoder;
        
        // 绑定资源
        auto diffuse = context.resolve(diffuse_texture);
        auto normal = context.resolve(normal_texture);
        
        // 设置视口和裁剪矩形
        cgpu_render_encoder_set_viewport(encoder, 
            0.0f, 0.0f, 1920.0f, 1080.0f, 0.0f, 1.0f);
        cgpu_render_encoder_set_scissor(encoder, 
            0, 0, 1920, 1080);
        
        // 绘制网格
        cgpu_render_encoder_bind_vertex_buffers(encoder, 1, &vb, nullptr, nullptr);
        cgpu_render_encoder_bind_index_buffer(encoder, ib, sizeof(uint32_t), 0);
        cgpu_render_encoder_draw_indexed(encoder, index_count, 0, 0);
    });
```

#### 计算 Pass

```cpp
auto compute_pass = graph->add_compute_pass(
    [=](RenderGraph& g, ComputePassBuilder& builder) {
        builder.set_name("post_process")
               .set_pipeline(compute_pipeline)
               
               // 读取输入
               .read("input_image", input_texture)
               
               // 读写输出
               .readwrite("output_image", output_texture)
               
               // 设置屏障（可选）
               .shader_stages(CGPU_SHADER_STAGE_COMPUTE);
    },
    
    [=](RenderGraph& g, ComputePassContext& context) {
        auto encoder = context.encoder;
        
        // 获取资源
        auto input = context.resolve(input_texture);
        auto output = context.resolve(output_texture);
        
        // 设置推送常量
        PostProcessParams params = { ... };
        cgpu_compute_encoder_push_constants(encoder, 
            compute_root_sig, "params", &params);
        
        // 分发计算
        cgpu_compute_encoder_dispatch(encoder, 
            (width + 7) / 8, (height + 7) / 8, 1);
    });
```

#### 拷贝 Pass

```cpp
auto copy_pass = graph->add_copy_pass(
    [=](RenderGraph& g, CopyPassBuilder& builder) {
        builder.set_name("mip_generation")
               .read(source_texture)
               .write(dest_texture);
    },
    
    [=](RenderGraph& g, CopyPassContext& context) {
        auto encoder = context.encoder;
        
        // 拷贝纹理区域
        CGPUTextureToTextureCopy copy_desc = {
            .src = context.resolve(source_texture),
            .src_subresource = { .mip_level = 0 },
            .dst = context.resolve(dest_texture),
            .dst_subresource = { .mip_level = 1 }
        };
        
        cgpu_copy_encoder_copy_texture_to_texture(encoder, &copy_desc);
    });
```

### 高级资源管理

#### 子资源访问

```cpp
// 访问特定 mip level
auto mip_1 = graph->get_texture_mip_level(texture_handle, 1);

// 访问特定数组层
auto layer_2 = graph->get_texture_array_layer(shadow_maps, 2);

// 访问特定子资源范围
TextureSubresourceRange range{
    .base_mip_level = 0,
    .mip_count = 3,
    .base_array_layer = 0,
    .layer_count = 1
};
auto subresource = graph->get_texture_subresource(texture, range);
```

#### 资源别名

```cpp
// 标记资源可以别名（共享内存）
auto temp_buffer1 = graph->create_texture(
    [=](RenderGraph& g, TextureBuilder& builder) {
        builder.set_name("temp1")
               .extent(1024, 1024)
               .format(CGPU_FORMAT_R8G8B8A8_UNORM)
               .allow_render_target()
               .aliasable();  // 允许别名
    });

// 如果生命周期不重叠，后端会自动共享内存
```

## 执行流程

### 编译和执行

```cpp
// 编译渲染图
graph->compile();

// 执行渲染图
graph->execute();

// 多帧执行模式
for (int frame = 0; frame < frame_count; ++frame) {
    // 更新动态资源
    graph->update_buffer(uniform_buffer, &frame_data);
    
    // 执行
    graph->execute();
    
    // 呈现
    cgpu_queue_present(graphics_queue, swapchain);
}
```

### 多帧并行

RenderGraph 支持多帧并行以提高 GPU 利用率：

```cpp
// 获取当前帧索引
uint32_t frame_index = graph->get_frame_index();

// 使用帧索引访问每帧资源
auto frame_buffer = per_frame_buffers[frame_index];
```

## 优化特性

### 自动屏障管理

RenderGraph 自动计算并插入必要的资源屏障：

```cpp
// 无需手动管理状态转换
// RenderGraph 会自动处理：
// - 纹理布局转换
// - 内存屏障
// - 执行依赖
```

### 资源池化

```cpp
// 配置资源池
graph->configure_pool(
    TexturePoolConfig{
        .max_textures = 1024,
        .memory_budget = 512 * 1024 * 1024  // 512MB
    });

// 资源会自动从池中分配和回收
```

### 内存别名优化

```cpp
// 启用内存别名
builder.enable_memory_aliasing(true);

// 系统会自动检测不重叠的资源并共享内存
// 例如：G-Buffer 和后处理临时纹理
```

## Blackboard 系统

Blackboard 提供全局命名资源注册表：

```cpp
// 注册资源到 Blackboard
graph->blackboard_add("main_depth", depth_buffer);
graph->blackboard_add("main_color", color_buffer);

// 在其他地方获取资源
auto depth = graph->blackboard_get_texture("main_depth");
auto color = graph->blackboard_get_texture("main_color");

// 用于跨系统共享资源
render_system->register_output("scene_color", final_color);
post_process_system->get_input("scene_color");
```

## 调试和分析

### GraphViz 可视化

```cpp
// 导出渲染图为 GraphViz 格式
graph->export_graphviz("render_graph.dot");

// 使用 Graphviz 工具生成图像
// dot -Tpng render_graph.dot -o render_graph.png
```

### GPU 标记

```cpp
// 自动生成 GPU 标记
graph->enable_gpu_markers(true);

// 在 GPU 调试器中可见：
// - Pass 名称
// - 资源名称
// - 执行时间
```

### 性能分析

```cpp
// 自定义 Profiler
class MyProfiler : public IRenderGraphProfiler {
    void begin_pass(const char* name) override {
        // 记录 Pass 开始
    }
    
    void end_pass(const char* name) override {
        // 记录 Pass 结束
    }
};

graph->set_profiler(std::make_unique<MyProfiler>());
```

## 最佳实践

### DO - 推荐做法

1. **使用描述性名称**
   ```cpp
   builder.set_name("shadow_map_pass")  // 好
   builder.set_name("pass1")            // 不好
   ```

2. **合理划分 Pass**
   ```cpp
   // 将相关操作组合在一个 Pass 中
   // 避免过于细粒度的 Pass
   ```

3. **复用资源**
   ```cpp
   // 使用资源池和别名优化内存使用
   builder.aliasable()
          .with_tags(kTemporaryResourceTag);
   ```

### DON'T - 避免做法

1. **避免过度依赖**
   ```cpp
   // 不要创建不必要的依赖链
   // 让 RenderGraph 自动推导依赖
   ```

2. **不要在 Execute 中分配资源**
   ```cpp
   // Execute 函数应该只记录命令
   // 资源创建应在 Setup 阶段完成
   ```

3. **避免循环依赖**
   ```cpp
   // Pass A 写入 -> Pass B 读取 -> Pass A 读取
   // 这会导致编译错误
   ```

## 高级用例

### 动态分辨率渲染

```cpp
class DynamicResolutionPass {
    TextureHandle color_buffer;
    
    void setup(RenderGraph& graph, uint32_t width, uint32_t height) {
        // 根据性能动态调整分辨率
        float scale = calculate_resolution_scale();
        uint32_t scaled_width = width * scale;
        uint32_t scaled_height = height * scale;
        
        color_buffer = graph.create_texture(
            [=](RenderGraph& g, TextureBuilder& b) {
                b.set_name("scaled_color")
                 .extent(scaled_width, scaled_height)
                 .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                 .allow_render_target();
            });
    }
};
```

### 多 GPU 渲染

```cpp
// 配置多 GPU
RenderGraphBuilder builder;
builder.with_device(device0)
       .with_device(device1)
       .enable_multi_gpu(true);

// 指定 Pass 在特定 GPU 执行
graph->add_render_pass(
    [=](RenderGraph& g, RenderPassBuilder& b) {
        b.set_name("gpu1_pass")
         .on_device(1);  // 在 GPU 1 执行
    },
    ...
);
```

### 异步计算

```cpp
// 在独立的计算队列执行
graph->add_async_compute_pass(
    [=](RenderGraph& g, ComputePassBuilder& b) {
        b.set_name("async_compute")
         .on_queue(compute_queue)
         .read("scene_depth", depth_buffer)
         .readwrite("ssao_buffer", ssao_texture);
    },
    [=](RenderGraph& g, ComputePassContext& ctx) {
        // SSAO 计算可以与主渲染并行
    });
```

## 集成示例

### 完整渲染管线

```cpp
class RenderPipeline {
    RenderGraph* graph;
    
    void setup() {
        // 1. 阴影图 Pass
        auto shadow_pass = graph->add_render_pass(...);
        
        // 2. G-Buffer Pass
        auto gbuffer_pass = graph->add_render_pass(...);
        
        // 3. 光照 Pass
        auto lighting_pass = graph->add_render_pass(...);
        
        // 4. 后处理 Pass
        auto post_process = graph->add_compute_pass(...);
        
        // 5. UI Pass
        auto ui_pass = graph->add_render_pass(...);
        
        // 6. 呈现 Pass
        auto present_pass = graph->add_present_pass(...);
    }
    
    void render(float delta_time) {
        // 更新每帧数据
        update_frame_data(delta_time);
        
        // 编译和执行
        graph->compile();
        graph->execute();
    }
};
```

## 总结

SakuraEngine 的 RenderGraph 系统提供了现代化的渲染框架，通过声明式 API 和自动资源管理大大简化了复杂渲染管线的实现。系统的前后端分离架构既保证了易用性，又提供了底层优化的灵活性。合理使用 RenderGraph 可以获得更好的性能、更少的 Bug 和更易维护的渲染代码。