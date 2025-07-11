# TimelinePhase 模块设计

## 概述
TimelinePhase是RenderGraph Phase系统的第一个阶段，负责多队列调度计算和跨队列同步需求分析。

## 核心职责
- **Pass分类**: 根据Pass类型和资源需求进行队列分类
- **队列调度**: 将Pass分配到Graphics/AsyncCompute/Copy队列
- **同步计算**: 分析跨队列依赖并计算同步需求
- **Fence分配**: 为跨队列同步创建CGPUFence对象

## 关键数据结构

### TimelineScheduleResult（输出）
```cpp
struct TimelineScheduleResult {
    skr::Vector<QueueScheduleInfo> queue_schedules;    // 各队列的Pass调度序列
    skr::Vector<SyncRequirement> sync_requirements;    // 跨队列同步需求
    skr::FlatHashMap<PassNode*, uint32_t> pass_queue_assignments; // Pass→队列映射
};
```

### SyncRequirement（同步需求）
```cpp
struct SyncRequirement {
    uint32_t signal_queue_index;    // 发信号的队列
    uint32_t wait_queue_index;      // 等待信号的队列
    PassNode* signal_after_pass;    // 在哪个Pass后发信号
    PassNode* wait_before_pass;     // 在哪个Pass前等待
    CGPUFenceId fence;              // 预分配的Fence对象
};
```

## 执行流程
1. **clear_frame_data()** - 清空上一帧数据
2. **analyze_dependencies()** - 分析Pass级别依赖关系
3. **classify_passes()** - Pass分类到preferred_queue Map
4. **assign_passes_to_queues()** - 一体化队列分配和Pass调度
5. **calculate_sync_requirements()** - 直接计算跨队列同步需求
6. **allocate_fences()** - 为同步需求分配CGPUFence对象

## 队列获取机制
- 通过RenderGraph接口获取队列：get_gfx_queue(), get_cmpt_queues(), get_cpy_queues()
- 不直接查询CGPU适配器，保持接口抽象
- 支持配置启用/禁用异步计算和拷贝队列

## Pass分类策略
- **Present/Render Pass** → Graphics队列（强制）
- **Copy Pass** → Copy队列（如果标记为lone且无Graphics依赖）
- **Compute Pass** → AsyncCompute队列（如果无Graphics资源依赖）
- **默认** → Graphics队列

## 关键修复：AsyncCompute调度优化
**问题**：之前所有有依赖的Pass都被强制分配到相同队列，导致AsyncCompute失效

**解决方案**：优化assign_passes_to_queues函数
- 优先保持Pass的首选队列分配（preferred_queue）
- 仅在目标队列不可用时才降级到依赖Pass的队列
- 通过Fence机制处理跨队列依赖，实现真正的并行调度

**结果验证**：
- ✅ 4个计算Pass成功分配到AsyncCompute队列
- ✅ 正确识别有图形资源依赖的Pass（如DeferredLightingPass依赖GBuffer）
- ✅ 2个跨队列同步需求通过Fence正确处理

## 生命周期管理
- **每帧开始**: clear_frame_data()清空所有动态数据
- **执行期间**: 重建Pass依赖和调度信息
- **帧结束**: 保留调度结果供其他Phase使用
- **Finalize**: 释放Fence资源和清理所有状态

## 接口设计
- **输入**: RenderGraph的Pass列表和队列信息
- **输出**: TimelineScheduleResult包含完整调度信息
- **配置**: TimelinePhaseConfig控制异步计算和拷贝队列启用

## 测试验证
**TimelineStressTest测试用例**：
- 复杂渲染管线：5个RenderPass + 5个ComputePass + 2个CopyPass + 1个PresentPass
- 验证多队列并行：AsyncCompute队列4个Pass，Graphics队列8个Pass
- 跨队列同步：2个Fence实现AsyncCompute↔Graphics同步

## 设计原则
- **单一职责**: 只负责调度计算，不处理命令执行
- **数据驱动**: 输出结构化调度结果供后续Phase使用
- **每帧重建**: 适应RenderGraph中Pass和资源的动态变化
- **性能优先**: 充分利用GPU多队列并行执行能力

## 重构优化记录

### 2025-07-11 精简化重构
**目标**: 提升代码精简性和鲁棒度，避免过度设计

**主要改进**:
1. **函数合并优化**
   - 合并 `initialize_queue_schedules()` 和 `assign_passes_to_best_queues()` 到 `assign_passes_to_queues()`
   - 减少函数调用开销，提高代码执行效率
   - 消除数据传递的中间环节

2. **同步计算简化**
   - 重写 `calculate_sync_requirements()` 为单一函数实现
   - 删除 `collect_cross_queue_dependencies()` 和 `compute_sync_requirements()` 函数
   - 使用 HashMap 直接去重跨队列依赖，避免复杂分组逻辑

3. **队列选择逻辑优化**
   - 简化 `select_best_queue_for_pass()` 的条件判断
   - 去除冗余的验证和回退逻辑
   - 采用早期返回模式，提高代码可读性

4. **调试输出精简**
   - 优化队列索引查找算法，使用 `skr::find` 替代循环
   - 减少临时字符串对象创建

**代码行数减少**: ~80行 → ~60行 (核心调度逻辑)
**函数数量减少**: 9个 → 6个 (删除3个辅助函数)

**效果验证**:
- 保持原有功能完整性
- 降低维护复杂度
- 提高代码执行效率
- 符合Phase最小单元设计原则