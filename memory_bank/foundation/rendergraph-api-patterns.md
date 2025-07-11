# RenderGraph API 使用模式

## 正确的API使用模式

### 资源句柄使用
- 纹理和缓冲区句柄直接传递，不需要显式调用视图方法
- 示例：`pass_builder.write(u8"depth", depth_texture, CGPU_LOAD_ACTION_CLEAR)`

### Pass执行函数参数类型
- Render Pass: `RenderPassContext`
- Compute Pass: `ComputePassContext`  
- Copy Pass: `CopyPassContext`

### RenderPassBuilder write方法
- 需要指定 `CGPU_LOAD_ACTION` 参数
- 示例：`builder.write(u8"color", rt, CGPU_LOAD_ACTION_CLEAR)`

### 资源读取方式
- 使用命名参数：`ctx.resolve(u8"color")`
- 不使用数值索引

## 错误的使用模式

### 避免的调用方式
- `texture.rtv()` - 纹理渲染目标视图
- `texture.srv()` - 纹理着色器资源视图
- `texture.dsv()` - 深度模板视图
- `buffer.uav()` - 缓冲区无序访问视图
- `buffer.range()` - 缓冲区范围方法

### 错误的参数类型
- 不使用 `RenderGraphFrameExecutor` 作为Pass执行函数参数

## 已解决的问题

### ComputePassBuilder缺失方法修复
**问题描述**：ComputePassBuilder类缺少必要的资源访问方法，导致计算Pass无法正确配置资源访问

**修复内容**：
- 添加 `read(uint32_t set, uint32_t binding, BufferHandle)` 方法
- 添加 `read(const char8_t* name, BufferHandle)` 方法  
- 添加 `readwrite(const char8_t* name, BufferHandle)` 方法
- 添加 `readwrite(uint32_t set, uint32_t binding, BufferHandle)` 方法
- 添加对应的 `BufferRangeHandle` 重载版本
- 添加对应的 `TextureHandle` 重载版本

**注意事项**：
- 测试用例中不应使用 `.range()` 方法，应直接使用 `BufferHandle`
- 方法签名与 `RenderPassBuilder` 保持一致性

### TimelinePhase AsyncCompute调度修复
**问题描述**：TimelinePhase中所有计算Pass都被错误分配到Graphics队列，AsyncCompute队列始终为空，无法实现多队列并行

**根本原因**：assign_passes_to_queues函数过度保守，强制将有依赖关系的Pass分配到相同队列

**修复方案**：
```cpp
// 优先保持preferred_queue，通过Fence处理跨队列同步
bool target_queue_available = false;
for (const auto& queue_info : schedule_result.queue_schedules) {
    if (queue_info.queue_type == preferred_queue) {
        target_queue_available = true;
        break;
    }
}

if (!target_queue_available && need_sync && !dependency_info[pass].dependencies.is_empty()) {
    // 只有当首选队列不可用时，才考虑分配到依赖Pass的队列
    auto* first_dep = dependency_info[pass].dependencies[0];
    auto dep_queue = (ERenderGraphQueueType)pass_to_queue[first_dep];
    if (can_run_on_queue(pass, dep_queue)) {
        assigned_queue = dep_queue;
    }
}
```

**验证结果**：
- ✅ AsyncCompute Queue: 4个Pass（FrustumCulling, ParticleSimulation, PhysicsSimulation, BloomCompute）
- ✅ Graphics Queue: 8个Pass（包含有图形资源依赖的DeferredLightingPass）
- ✅ 跨队列同步：2个Fence正确处理AsyncCompute↔Graphics同步

**影响**：实现真正的多队列并行执行，充分利用现代GPU异步计算能力

## 最佳实践

### RenderGraph创建
- 测试环境使用：`frontend_only()` 方法创建RenderGraph实例

### Pass命名
- 使用描述性字符串命名Pass
- 示例：`u8"ClearPass"`, `u8"GeometryPass"`

### 资源依赖标识
- 使用命名字符串标识符管理资源依赖
- 示例：`u8"color"`, `u8"depth"`, `u8"gbuffer"`

### 典型使用流程
```cpp
// 1. 创建RenderGraph
auto render_graph = RenderGraph::frontend_only();

// 2. 导入资源
auto rt = render_graph->create_texture(...);

// 3. 添加Pass
render_graph->add_render_pass(
    u8"PassName",
    [&](RenderPassBuilder& builder) {
        builder.write(u8"output", rt, CGPU_LOAD_ACTION_CLEAR);
    },
    [=](RenderPassContext& ctx) {
        auto output = ctx.resolve(u8"output");
        // Pass逻辑
    }
);

// 4. 编译和执行
render_graph->compile();
render_graph->execute();
```