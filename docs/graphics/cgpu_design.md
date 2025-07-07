# SakuraEngine CGPU 图形抽象层

## 概述

CGPU (Cross-platform GPU) 是 SakuraEngine 的高性能跨平台图形 API 抽象层。它提供了一个薄而强大的抽象，统一了 D3D12、Vulkan 和 Metal 等现代图形 API，同时保持接近原生的性能和功能访问。

## 设计理念

### 核心原则

1. **零开销抽象** - 通过 C 接口和直接映射避免虚函数开销
2. **显式控制** - 暴露底层硬件特性，让开发者完全控制
3. **现代 GPU 优先** - 围绕 GPU 并行、多队列等现代特性设计
4. **可扩展性** - 支持厂商扩展和平台特定功能

### 为什么选择 C 接口

```c
// CGPU 使用纯 C 接口
typedef struct CGPUDevice* CGPUDeviceId;
typedef struct CGPUTexture* CGPUTextureId;

// 而不是 C++ 类
// class IGPUDevice { virtual void foo() = 0; };
```

优势：
- ABI 稳定性 - C 接口保证跨编译器兼容
- 零虚函数开销 - 通过函数表而非虚表调度
- 易于绑定 - 可以轻松绑定到其他语言
- 缓存友好 - 对象大小可控，避免虚表指针

## 对象模型

### 层次结构

```
CGPUInstance
    │
    ├── CGPUAdapter (物理 GPU)
    │       │
    │       └── CGPUDevice (逻辑设备)
    │               │
    │               ├── CGPUQueue (命令队列)
    │               ├── Resources (资源)
    │               └── Pipeline Objects (管线对象)
    │
    └── CGPUAdapter (另一个 GPU)
```

### 实例和适配器

```c
// 创建实例
CGPUInstanceDescriptor instance_desc = {
    .backend = CGPU_BACKEND_VULKAN,
    .enable_debug_layer = true,
    .enable_gpu_based_validation = true,
    .enable_set_name = true  // 启用对象命名（调试用）
};
CGPUInstanceId instance = cgpu_create_instance(&instance_desc);

// 枚举适配器
uint32_t adapter_count = 0;
cgpu_enumerate_adapters(instance, nullptr, &adapter_count);

CGPUAdapterId adapters[adapter_count];
cgpu_enumerate_adapters(instance, adapters, &adapter_count);

// 查询适配器信息
CGPUAdapterDetail detail;
cgpu_query_adapter_detail(adapters[0], &detail);

// 选择高性能独显
for (uint32_t i = 0; i < adapter_count; i++) {
    cgpu_query_adapter_detail(adapters[i], &detail);
    if (detail.is_discrete) {
        selected_adapter = adapters[i];
        break;
    }
}
```

### 设备和队列

```c
// 创建设备
CGPUQueueGroupDescriptor queue_groups[] = {
    {
        .queue_type = CGPU_QUEUE_TYPE_GRAPHICS,
        .queue_count = 1
    },
    {
        .queue_type = CGPU_QUEUE_TYPE_COMPUTE,
        .queue_count = 2  // 两个计算队列
    },
    {
        .queue_type = CGPU_QUEUE_TYPE_TRANSFER,
        .queue_count = 1
    }
};

CGPUDeviceDescriptor device_desc = {
    .queue_groups = queue_groups,
    .queue_group_count = 3
};

CGPUDeviceId device = cgpu_create_device(selected_adapter, &device_desc);

// 获取队列
CGPUQueueId gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
CGPUQueueId compute_queue0 = cgpu_get_queue(device, CGPU_QUEUE_TYPE_COMPUTE, 0);
CGPUQueueId compute_queue1 = cgpu_get_queue(device, CGPU_QUEUE_TYPE_COMPUTE, 1);
```

## 资源管理

### Buffer 创建和管理

```c
// 创建顶点缓冲区
CGPUBufferDescriptor vb_desc = {
    .name = u8"vertex_buffer",
    .size = sizeof(Vertex) * vertex_count,
    .descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER,
    .memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
    .flags = CGPU_BCF_PERSISTENT_MAP_BIT  // 持久映射
};
CGPUBufferId vertex_buffer = cgpu_create_buffer(device, &vb_desc);

// 创建动态常量缓冲区
CGPUBufferDescriptor cb_desc = {
    .name = u8"frame_constants",
    .size = sizeof(FrameConstants),
    .descriptors = CGPU_RESOURCE_TYPE_UNIFORM_BUFFER,
    .memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU,
    .flags = CGPU_BCF_PERSISTENT_MAP_BIT
};
CGPUBufferId constant_buffer = cgpu_create_buffer(device, &cb_desc);

// 更新动态缓冲区
FrameConstants* mapped = (FrameConstants*)cgpu_buffer_get_mapped_data(constant_buffer);
*mapped = frame_data;  // 直接写入，无需 map/unmap
```

### Texture 创建和管理

```c
// 创建 2D 纹理
CGPUTextureDescriptor tex_desc = {
    .name = u8"color_texture",
    .width = 1920,
    .height = 1080,
    .depth = 1,
    .array_size = 1,
    .mip_levels = 1,
    .format = CGPU_FORMAT_R8G8B8A8_UNORM,
    .sample_count = CGPU_SAMPLE_COUNT_1,
    .descriptors = CGPU_RESOURCE_TYPE_TEXTURE,
    .flags = CGPU_TCF_USAGE_SAMPLED_IMAGE | CGPU_TCF_USAGE_UNORDERED_ACCESS,
    .native_handle = nullptr  // 可以导入外部纹理
};
CGPUTextureId texture = cgpu_create_texture(device, &tex_desc);

// 创建纹理视图（用于特定 mip 或数组层）
CGPUTextureViewDescriptor view_desc = {
    .texture = texture,
    .format = CGPU_FORMAT_R8G8B8A8_UNORM,
    .array_layer_count = 1,
    .base_array_layer = 0,
    .mip_level_count = 1,
    .base_mip_level = 0,
    .aspects = CGPU_TVA_COLOR
};
CGPUTextureViewId texture_view = cgpu_create_texture_view(device, &view_desc);
```

### 高级内存管理

```c
// 创建自定义内存池
CGPUMemoryPoolDescriptor pool_desc = {
    .name = u8"texture_pool",
    .block_size = 64 * 1024 * 1024,  // 64MB 块
    .min_block_count = 4,
    .max_block_count = 16
};
CGPUMemoryPoolId memory_pool = cgpu_create_memory_pool(device, &pool_desc);

// 从池中分配纹理
tex_desc.memory_pool = memory_pool;
CGPUTextureId pooled_texture = cgpu_create_texture(device, &tex_desc);

// Tiled Resource (稀疏纹理)
CGPUTiledTextureDescriptor tiled_desc = {
    .width = 16384,
    .height = 16384,
    .format = CGPU_FORMAT_R8G8B8A8_UNORM,
    .tile_size = 256,  // 256x256 tiles
};
CGPUTiledTextureId tiled_texture = cgpu_create_tiled_texture(device, &tiled_desc);

// 映射 tile
CGPUTileMapping mapping = {
    .texture = tiled_texture,
    .x = 0, .y = 0,
    .width = 256, .height = 256,
    .memory_offset = 0
};
cgpu_update_tile_mappings(queue, &mapping, 1);
```

## 渲染管线

### Root Signature (资源布局)

```c
// 定义着色器资源布局
CGPUShaderResource resources[] = {
    // 常量缓冲区
    {
        .name = u8"FrameData",
        .type = CGPU_RESOURCE_TYPE_UNIFORM_BUFFER,
        .set = 0,
        .binding = 0,
        .stages = CGPU_SHADER_STAGE_VERT | CGPU_SHADER_STAGE_FRAG
    },
    // 纹理
    {
        .name = u8"diffuseTexture",
        .type = CGPU_RESOURCE_TYPE_TEXTURE,
        .set = 0,
        .binding = 1,
        .stages = CGPU_SHADER_STAGE_FRAG
    },
    // 采样器
    {
        .name = u8"linearSampler",
        .type = CGPU_RESOURCE_TYPE_SAMPLER,
        .set = 0,
        .binding = 2,
        .stages = CGPU_SHADER_STAGE_FRAG
    }
};

// 静态采样器
CGPUStaticSamplerDescriptor static_samplers[] = {
    {
        .name = u8"linearSampler",
        .binding = 2,
        .filter = CGPU_FILTER_TYPE_LINEAR,
        .address_u = CGPU_ADDRESS_MODE_REPEAT,
        .address_v = CGPU_ADDRESS_MODE_REPEAT,
        .address_w = CGPU_ADDRESS_MODE_REPEAT
    }
};

// Push Constants
CGPUPushConstantDescriptor push_constants = {
    .name = u8"DrawData",
    .shader_stages = CGPU_SHADER_STAGE_VERT,
    .size = sizeof(DrawPushConstants)
};

// 创建 Root Signature
CGPURootSignatureDescriptor rs_desc = {
    .name = u8"main_root_sig",
    .shaders = shader_ptrs,
    .shader_count = 2,
    .static_samplers = static_samplers,
    .static_sampler_count = 1,
    .push_constants = &push_constants,
    .push_constant_count = 1
};
CGPURootSignatureId root_sig = cgpu_create_root_signature(device, &rs_desc);
```

### 渲染管线状态

```c
// 顶点布局
CGPUVertexAttribute vertex_attrs[] = {
    { // 位置
        .attribute = u8"POSITION",
        .format = CGPU_FORMAT_R32G32B32_SFLOAT,
        .binding = 0,
        .offset = offsetof(Vertex, position)
    },
    { // 法线
        .attribute = u8"NORMAL",
        .format = CGPU_FORMAT_R32G32B32_SFLOAT,
        .binding = 0,
        .offset = offsetof(Vertex, normal)
    },
    { // UV
        .attribute = u8"TEXCOORD0",
        .format = CGPU_FORMAT_R32G32_SFLOAT,
        .binding = 0,
        .offset = offsetof(Vertex, uv)
    }
};

CGPUVertexLayout vertex_layout = {
    .attributes = vertex_attrs,
    .attribute_count = 3,
    .strides = { sizeof(Vertex) },
    .rates = { CGPU_INPUT_RATE_VERTEX }
};

// 混合状态
CGPUBlendStateDescriptor blend_state = {
    .alpha_to_coverage = false,
    .independent_blend = false,
    .blend_states = {
        {
            .blend_enabled = true,
            .src_color = CGPU_BLEND_FACTOR_SRC_ALPHA,
            .dst_color = CGPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .color_op = CGPU_BLEND_OP_ADD,
            .src_alpha = CGPU_BLEND_FACTOR_ONE,
            .dst_alpha = CGPU_BLEND_FACTOR_ZERO,
            .alpha_op = CGPU_BLEND_OP_ADD,
            .color_write_mask = CGPU_COLOR_MASK_ALL
        }
    }
};

// 创建图形管线
CGPURenderPipelineDescriptor pipeline_desc = {
    .name = u8"main_pipeline",
    .root_signature = root_sig,
    .vertex_shader = { .library = vertex_shader },
    .fragment_shader = { .library = fragment_shader },
    .vertex_layout = &vertex_layout,
    .blend_state = &blend_state,
    .depth_state = &depth_state,
    .rasterizer_state = &rasterizer_state,
    .render_target_count = 1,
    .color_formats = { CGPU_FORMAT_R8G8B8A8_UNORM },
    .depth_stencil_format = CGPU_FORMAT_D32_SFLOAT,
    .sample_count = CGPU_SAMPLE_COUNT_1,
    .primitive_topology = CGPU_PRIM_TOPO_TRI_LIST
};
CGPURenderPipelineId pipeline = cgpu_create_render_pipeline(device, &pipeline_desc);
```

## 命令录制

### 基本渲染流程

```c
// 获取命令缓冲区
CGPUCommandPoolId cmd_pool = cgpu_create_command_pool(gfx_queue, nullptr);
CGPUCommandBufferId cmd = cgpu_create_command_buffer(cmd_pool, nullptr);

// 开始录制
cgpu_cmd_begin(cmd);

// 资源屏障 - 转换到渲染目标状态
CGPUTextureBarrier rt_barrier = {
    .texture = render_target,
    .src_state = CGPU_RESOURCE_STATE_UNDEFINED,
    .dst_state = CGPU_RESOURCE_STATE_RENDER_TARGET
};
CGPUResourceBarrierDescriptor barriers = {
    .texture_barriers = &rt_barrier,
    .texture_barrier_count = 1
};
cgpu_cmd_resource_barrier(cmd, &barriers);

// 开始渲染 Pass
CGPUColorAttachment color_attachments[] = {
    {
        .view = render_target_view,
        .load_action = CGPU_LOAD_ACTION_CLEAR,
        .store_action = CGPU_STORE_ACTION_STORE,
        .clear_color = { 0.2f, 0.3f, 0.4f, 1.0f }
    }
};

CGPUDepthStencilAttachment depth_attachment = {
    .view = depth_view,
    .depth_load_action = CGPU_LOAD_ACTION_CLEAR,
    .depth_store_action = CGPU_STORE_ACTION_STORE,
    .clear_depth = 1.0f,
    .stencil_load_action = CGPU_LOAD_ACTION_DONT_CARE,
    .stencil_store_action = CGPU_STORE_ACTION_DONT_CARE
};

CGPURenderPassDescriptor pass_desc = {
    .name = u8"main_pass",
    .color_attachments = color_attachments,
    .color_attachment_count = 1,
    .depth_stencil = &depth_attachment
};

CGPURenderPassEncoderId encoder = cgpu_cmd_begin_render_pass(cmd, &pass_desc);

// 设置管线和资源
cgpu_render_encoder_bind_pipeline(encoder, pipeline);
cgpu_render_encoder_set_viewport(encoder, 0, 0, 1920, 1080, 0.0f, 1.0f);
cgpu_render_encoder_set_scissor(encoder, 0, 0, 1920, 1080);

// 绑定顶点缓冲区
cgpu_render_encoder_bind_vertex_buffers(encoder, 1, &vertex_buffer, &vb_stride, nullptr);
cgpu_render_encoder_bind_index_buffer(encoder, index_buffer, sizeof(uint32_t), 0);

// 绑定描述符集
cgpu_render_encoder_bind_descriptor_set(encoder, descriptor_set);

// Push Constants
cgpu_render_encoder_push_constants(encoder, root_sig, u8"DrawData", &push_data);

// 绘制
cgpu_render_encoder_draw_indexed(encoder, index_count, 0, 0);

// 结束渲染 Pass
cgpu_cmd_end_render_pass(cmd, encoder);

// 结束命令录制
cgpu_cmd_end(cmd);

// 提交
CGPUQueueSubmitDescriptor submit_desc = {
    .cmds = &cmd,
    .cmds_count = 1,
    .signal_fence = render_fence
};
cgpu_submit_queue(gfx_queue, &submit_desc);
```

### 计算 Pass

```c
// 计算 Pass 示例
CGPUComputePassDescriptor compute_pass_desc = {
    .name = u8"compute_pass"
};
CGPUComputePassEncoderId compute_encoder = cgpu_cmd_begin_compute_pass(cmd, &compute_pass_desc);

// 绑定计算管线
cgpu_compute_encoder_bind_pipeline(compute_encoder, compute_pipeline);

// 绑定资源
cgpu_compute_encoder_bind_descriptor_set(compute_encoder, compute_desc_set);

// 分发计算
cgpu_compute_encoder_dispatch(compute_encoder, (width + 7) / 8, (height + 7) / 8, 1);

cgpu_cmd_end_compute_pass(cmd, compute_encoder);
```

## 高级特性

### 描述符管理

```c
// 创建描述符集
CGPUDescriptorSetDescriptor set_desc = {
    .root_signature = root_sig,
    .set_index = 0
};
CGPUDescriptorSetId descriptor_set = cgpu_create_descriptor_set(device, &set_desc);

// 更新描述符
CGPUDescriptorData updates[] = {
    { // 常量缓冲区
        .name = u8"FrameData",
        .buffers = &constant_buffer,
        .count = 1
    },
    { // 纹理
        .name = u8"diffuseTexture",
        .textures = &texture_view,
        .count = 1
    }
};
cgpu_update_descriptor_set(descriptor_set, updates, 2);

// 高级绑定表 API
CGPUXBindTableId bind_table = cgpux_create_bind_table(device, root_sig);
cgpux_bind_table_update(bind_table, u8"FrameData", &constant_buffer);
cgpux_bind_table_update(bind_table, u8"diffuseTexture", &texture_view);
```

### 多队列同步

```c
// 创建信号量
CGPUSemaphoreId semaphore = cgpu_create_semaphore(device);

// 图形队列提交（信号）
CGPUQueueSubmitDescriptor gfx_submit = {
    .cmds = &gfx_cmd,
    .cmds_count = 1,
    .signal_semaphores = &semaphore,
    .signal_semaphore_count = 1
};
cgpu_submit_queue(gfx_queue, &gfx_submit);

// 计算队列提交（等待）
CGPUQueueSubmitDescriptor compute_submit = {
    .cmds = &compute_cmd,
    .cmds_count = 1,
    .wait_semaphores = &semaphore,
    .wait_semaphore_count = 1
};
cgpu_submit_queue(compute_queue, &compute_submit);
```

### DirectStorage 集成

```c
// 创建 DirectStorage 队列
CGPUDStorageQueueDescriptor dstorage_desc = {
    .name = u8"texture_streaming",
    .capacity = CGPU_DSTORAGE_QUEUE_CAPACITY_DEFAULT,
    .priority = CGPU_DSTORAGE_PRIORITY_NORMAL,
    .source_type = CGPU_DSTORAGE_SOURCE_FILE,
    .device = device
};
CGPUDStorageQueueId dstorage_queue = cgpu_create_dstorage_queue(&dstorage_desc);

// 直接加载压缩纹理到 GPU
CGPUDStorageBufferIODescriptor io_desc = {
    .path = u8"textures/large_texture.dds.lz4",
    .buffer = staging_buffer,
    .offset = 0,
    .size = file_size,
    .compression = CGPU_DSTORAGE_COMPRESSION_LZ4,
    .source_type = CGPU_DSTORAGE_SOURCE_FILE
};
cgpu_dstorage_enqueue_buffer_request(dstorage_queue, &io_desc);

// 提交并等待
cgpu_dstorage_submit(dstorage_queue, dstorage_fence);
cgpu_wait_fences(&dstorage_fence, 1);
```

## 性能最佳实践

### DO - 推荐做法

1. **批量资源屏障**
   ```c
   // 好：批量屏障
   CGPUTextureBarrier barriers[10];
   // ... 填充屏障
   cgpu_cmd_resource_barrier(cmd, &barrier_desc);
   
   // 避免：单个屏障
   for (int i = 0; i < 10; i++) {
       cgpu_cmd_resource_barrier(cmd, &single_barrier);
   }
   ```

2. **使用持久映射**
   ```c
   // 动态缓冲区使用持久映射
   .flags = CGPU_BCF_PERSISTENT_MAP_BIT
   ```

3. **并行命令录制**
   ```c
   // 使用多个命令池并行录制
   #pragma omp parallel for
   for (int i = 0; i < thread_count; i++) {
       record_commands(cmd_pools[i]);
   }
   ```

### DON'T - 避免做法

1. **避免频繁的小更新**
   ```c
   // 不好：每帧创建描述符集
   CGPUDescriptorSetId set = cgpu_create_descriptor_set(...);
   
   // 好：复用和更新
   cgpu_update_descriptor_set(existing_set, ...);
   ```

2. **不要忽略队列类型**
   ```c
   // 使用专用传输队列进行上传
   // 而不是在图形队列上做所有事情
   ```

## 总结

CGPU 提供了一个强大而灵活的图形 API 抽象层，既保持了接近原生的性能，又提供了跨平台的便利性。通过显式的资源管理、现代的 GPU 特性支持和零开销的 C 接口设计，CGPU 成为构建高性能渲染引擎的理想基础。