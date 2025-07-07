# SakuraEngine ECS (Sugoi) 系统

## 概述

Sugoi 是 SakuraEngine 的高性能 Entity Component System (ECS) 实现，采用数据导向设计，为游戏逻辑提供缓存友好、易于并行化的架构。

系统名称 "Sugoi" 来自日语「すごい」，意为"厉害"。

## 核心概念

### 基础组件

#### Entity (实体)
实体是一个轻量级的标识符，本质上是一个 64 位整数：

```cpp
// sugoi_entity_t 结构
// 高32位: 版本号（用于检测实体复用）
// 低32位: 实体ID
sugoi_entity_t entity = storage->new_entity();

// 访问实体信息
uint32_t id = e_id(entity);        // 获取ID
uint32_t version = e_version(entity); // 获取版本号
```

#### Component (组件)
组件是纯数据结构，不包含逻辑：

```cpp
// 定义组件
sreflect_struct("guid": "12345678-...", "ecs.comp": true)
struct PositionComponent {
    float x, y, z;
};

sreflect_struct("guid": "87654321-...", "ecs.comp": true)
struct VelocityComponent {
    float vx, vy, vz;
};

// 获取组件类型ID
auto pos_type = sugoi_id_of<PositionComponent>::get();
```

#### Chunk (数据块)
Chunk 是 ECS 数据的基本存储单元，同一 Archetype 的实体存储在一起：

```cpp
// Chunk 特性
// - 固定大小（通常 16KB）
// - 内存连续，缓存友好
// - 包含实体数组和各组件数组
// - 支持时间戳追踪变更
```

#### Archetype (原型)
Archetype 定义了一组组件类型的组合：

```cpp
// Archetype 包含的信息
struct sugoi_archetype_t {
    // 组件类型列表
    sugoi_type_index_t* types;
    uint32_t type_count;
    
    // 组件在 chunk 中的偏移
    uint32_t* offsets;
    
    // 组件大小和对齐
    uint32_t* sizes;
    uint32_t* aligns;
};
```

#### Group (组)
Group 是相同 Archetype 的 Chunk 集合：

```cpp
// Group 管理
// - 分配和回收 chunks
// - 维护空闲列表
// - 支持共享组件（元实体）
```

### 存储系统

```cpp
// 创建 ECS 存储
sugoi_storage_t* storage = sugoiS_create();

// 配置选项
sugoi_storage_config_t config = {
    .enable_guid = true,        // 启用 GUID 组件
    .enable_timestamps = true,  // 启用时间戳追踪
    .enable_dirty = true        // 启用脏标记
};
```

## 基本操作

### 创建和销毁实体

```cpp
// 创建实体
sugoi_entity_t entity = sugoiS_create_entity(storage);

// 批量创建
sugoi_entity_t entities[100];
sugoiS_create_entities(storage, entities, 100);

// 销毁实体
sugoiS_destroy_entity(storage, entity);

// 检查实体是否存活
bool alive = sugoiS_is_entity_alive(storage, entity);
```

### 添加和移除组件

```cpp
// 添加组件
PositionComponent pos = {10.0f, 20.0f, 30.0f};
sugoiS_add_component(storage, entity, 
    sugoi_id_of<PositionComponent>::get(), &pos);

// 移除组件  
sugoiS_remove_component(storage, entity,
    sugoi_id_of<PositionComponent>::get());

// 检查组件
bool has_pos = sugoiS_has_component(storage, entity,
    sugoi_id_of<PositionComponent>::get());

// 获取组件
PositionComponent* pos_ptr = (PositionComponent*)
    sugoiS_get_component(storage, entity, 
        sugoi_id_of<PositionComponent>::get());
```

### 批量操作

```cpp
// 批量添加组件
sugoi_delta_t* delta = sugoiS_create_delta(storage);

// 设置组件
sugoiD_set_component(delta, entity, 
    sugoi_id_of<PositionComponent>::get(), &pos);
sugoiD_set_component(delta, entity,
    sugoi_id_of<VelocityComponent>::get(), &vel);

// 应用变更
sugoiS_apply_delta(storage, delta);
sugoiS_destroy_delta(delta);
```

## 查询系统

### Query Builder API

使用类型安全的 API 构建查询：

```cpp
// 创建查询
auto query = storage->new_query()
    .ReadWriteAll<PositionComponent>()      // 读写位置
    .ReadAll<VelocityComponent>()           // 只读速度
    .OptionalAll<ScaleComponent>()          // 可选缩放
    .NoneAll<DisabledTag>()                 // 排除禁用的
    .commit();

// 遍历查询结果
query.foreach([](PositionComponent& pos, 
                const VelocityComponent& vel,
                ScaleComponent* scale) {
    // 更新位置
    pos.x += vel.vx;
    pos.y += vel.vy;
    pos.z += vel.vz;
    
    // 如果有缩放组件
    if (scale) {
        // 处理缩放
    }
});
```

### 字符串 DSL 查询

使用字符串定义查询，支持运行时动态创建：

```cpp
// DSL 语法：
// [inout] - 读写访问
// [in]    - 只读访问
// ?       - 可选组件
// !       - 排除组件

auto query = sugoiQ_from_literal(storage,
    u8"[inout]PositionComponent, "
    u8"[in]VelocityComponent, "
    u8"[in]?ScaleComponent, "
    u8"!DisabledTag");
```

### 高级查询特性

#### 过滤器

```cpp
// 添加自定义过滤器
query.filter([](sugoi_chunk_view_t* view) {
    // 返回 true 包含这个 chunk
    return view->count > 10;
});
```

#### 排序

```cpp
// 按组件值排序
query.order_by<PositionComponent>(
    [](const PositionComponent& a, const PositionComponent& b) {
        return a.x < b.x;
    });
```

#### 分组

```cpp
// 按共享组件分组
query.group_by<TeamComponent>();
```

## 并行处理

### 自动并行

```cpp
// 并行遍历查询结果
sugoiJ_schedule_ecs(query.get_sugoi_query(), 
    64,     // 批处理大小
    [](void* userdata, sugoi_query_t* query, 
       sugoi_chunk_view_t* view, uint32_t start, uint32_t end) {
        // 处理 [start, end) 范围的实体
        auto positions = view->get_component_array<PositionComponent>();
        auto velocities = view->get_component_array<VelocityComponent>();
        
        for (uint32_t i = start; i < end; ++i) {
            positions[i].x += velocities[i].vx;
            positions[i].y += velocities[i].vy;
            positions[i].z += velocities[i].vz;
        }
    },
    nullptr,  // userdata
    nullptr,  // init callback
    nullptr,  // teardown callback
    nullptr,  // resources
    nullptr   // counter
);
```

### 手动并行控制

```cpp
// 获取 chunk 视图进行手动并行
query.foreach_chunk([](sugoi_chunk_view_t* view) {
    // 每个 chunk 可以在不同线程处理
    auto count = view->count;
    auto positions = view->get_component_array<PositionComponent>();
    
    // 使用你自己的线程池
    thread_pool.enqueue([positions, count] {
        for (uint32_t i = 0; i < count; ++i) {
            // 处理数据
        }
    });
});
```

## 特殊组件

### GUID 组件
用于给实体分配全局唯一标识符：

```cpp
// 自动为所有实体添加 GUID
storage->enable_guid();

// 通过 GUID 查找实体
skr_guid_t guid = ...;
sugoi_entity_t entity = sugoiS_get_entity_by_guid(storage, guid);
```

### Dirty 组件
追踪组件修改状态：

```cpp
// 启用脏标记
storage->enable_dirty();

// 检查组件是否被修改
bool is_dirty = sugoiS_is_component_dirty(storage, entity,
    sugoi_id_of<PositionComponent>::get());

// 清除脏标记
sugoiS_clear_dirty(storage, entity);
```

### Mask 组件
用于快速过滤实体：

```cpp
// 设置掩码
uint32_t mask = MASK_VISIBLE | MASK_ACTIVE;
sugoiS_set_mask(storage, entity, mask);

// 查询时过滤
query.with_mask(MASK_VISIBLE);
```

## 共享组件

共享组件通过元实体实现，多个实体可以引用同一份数据：

```cpp
// 定义共享组件
sreflect_struct("ecs.comp": true, "ecs.shared": true)
struct TeamComponent {
    uint32_t team_id;
    SkrString team_name;
};

// 创建元实体
sugoi_entity_t meta = sugoiS_create_entity(storage);
TeamComponent team = {1, u8"Red Team"};
sugoiS_add_component(storage, meta, 
    sugoi_id_of<TeamComponent>::get(), &team);

// 多个实体共享
sugoiS_set_meta(storage, entity1, meta);
sugoiS_set_meta(storage, entity2, meta);
```

## 性能优化

### 内存布局优化

1. **组件打包**
   ```cpp
   // 将经常一起访问的组件放在同一个 archetype
   sreflect_struct("ecs.comp": true)
   struct TransformBundle {
       float3 position;
       quaternion rotation;
       float3 scale;
   };
   ```

2. **避免稀疏组件**
   ```cpp
   // 不好：很少使用的大组件
   struct RareHugeComponent {
       float data[1024];
   };
   
   // 好：使用间接引用
   struct RareDataRef {
       uint32_t data_id;  // 在外部存储查找
   };
   ```

### 查询优化

1. **缓存查询对象**
   ```cpp
   // 避免每帧创建查询
   class MovementSystem {
       sugoi_query_t* query;
       
       void initialize() {
           query = storage->new_query()
               .ReadWriteAll<PositionComponent>()
               .ReadAll<VelocityComponent>()
               .commit().detach();
       }
   };
   ```

2. **使用 chunk 迭代**
   ```cpp
   // 批量处理整个 chunk 更高效
   query.foreach_chunk([](auto* view) {
       // SIMD 友好的处理
   });
   ```

### 并行优化

1. **合理的批处理大小**
   ```cpp
   // 根据工作负载调整
   // 轻量级工作：较大批次（128-256）
   // 重量级工作：较小批次（16-64）
   sugoiJ_schedule_ecs(query, 64, callback, ...);
   ```

2. **减少同步开销**
   ```cpp
   // 使用只读查询避免锁
   auto read_query = storage->new_query()
       .ReadAll<PositionComponent>()
       .commit();
   ```

## 最佳实践

### DO - 推荐做法

1. **数据导向设计**
   - 组件只包含数据，不包含逻辑
   - 系统处理组件数据
   - 优先考虑批处理

2. **合理划分 Archetype**
   - 相关组件组合在一起
   - 避免过于细粒度的组件
   - 考虑查询模式

3. **利用并行性**
   - 使用并行调度 API
   - 设计无冲突的系统
   - 合理设置批处理大小

### DON'T - 避免做法

1. **避免频繁的结构变更**
   - 添加/移除组件会导致数据移动
   - 使用标记组件代替频繁变更
   - 批量处理结构变更

2. **不要在组件中存储逻辑**
   - 组件应该是 POD 类型
   - 避免虚函数和复杂构造
   - 逻辑放在系统中

3. **避免单实体操作**
   - ECS 为批处理优化
   - 尽量使用查询批量处理
   - 缓存组件指针要小心

## 调试技巧

### 实体检查

```cpp
// 打印实体信息
void debug_entity(sugoi_storage_t* storage, sugoi_entity_t entity) {
    if (!sugoiS_is_entity_alive(storage, entity)) {
        SKR_LOG_ERROR(u8"Entity is dead!");
        return;
    }
    
    SKR_LOG_INFO(u8"Entity: ID=%u, Version=%u", 
        e_id(entity), e_version(entity));
    
    // 列出所有组件
    auto archetype = sugoiS_get_entity_archetype(storage, entity);
    for (uint32_t i = 0; i < archetype->type_count; ++i) {
        auto type = archetype->types[i];
        SKR_LOG_INFO(u8"  Component: %s", 
            sugoiT_get_name(type));
    }
}
```

### 性能分析

```cpp
// 测量查询性能
auto start = skr_now();
uint32_t entity_count = 0;

query.foreach([&entity_count](...) {
    entity_count++;
});

auto elapsed = skr_now() - start;
SKR_LOG_INFO(u8"Processed %u entities in %.2fms",
    entity_count, elapsed * 1000.0);
```

### 内存统计

```cpp
// 获取存储统计信息
sugoi_storage_stats_t stats;
sugoiS_get_stats(storage, &stats);

SKR_LOG_INFO(u8"Entities: %u", stats.entity_count);
SKR_LOG_INFO(u8"Chunks: %u", stats.chunk_count);
SKR_LOG_INFO(u8"Archetypes: %u", stats.archetype_count);
SKR_LOG_INFO(u8"Memory: %.2fMB", stats.memory_usage / 1024.0 / 1024.0);
```

## 高级特性

### 时间戳系统

追踪组件最后修改时间：

```cpp
// 启用时间戳
storage->enable_timestamps();

// 查询最近修改的实体
auto query = storage->new_query()
    .ReadAll<PositionComponent>()
    .changed_since(last_frame_timestamp)
    .commit();
```

### 事件系统

响应 ECS 变更：

```cpp
// 注册回调
sugoiS_register_callback(storage, 
    SUGOI_CALLBACK_ON_ENTITY_CREATE,
    [](sugoi_storage_t* storage, sugoi_entity_t entity) {
        SKR_LOG_INFO(u8"Entity created: %u", e_id(entity));
    });

sugoiS_register_callback(storage,
    SUGOI_CALLBACK_ON_COMPONENT_ADD,
    [](sugoi_storage_t* storage, sugoi_entity_t entity,
       sugoi_type_index_t type) {
        SKR_LOG_INFO(u8"Component added: %s", 
            sugoiT_get_name(type));
    });
```

### 序列化支持

保存和加载 ECS 状态：

```cpp
// 序列化到文件
sugoi_serializer_t* serializer = sugoiS_create_serializer(storage);
sugoiS_serialize_to_file(serializer, u8"save.ecs");
sugoiS_destroy_serializer(serializer);

// 从文件加载
sugoi_deserializer_t* deserializer = 
    sugoiS_create_deserializer(storage);
sugoiS_deserialize_from_file(deserializer, u8"save.ecs");
sugoiS_destroy_deserializer(deserializer);
```

## 与其他系统集成

### 渲染系统集成

```cpp
// 收集可渲染实体
auto render_query = storage->new_query()
    .ReadAll<TransformComponent>()
    .ReadAll<MeshComponent>()
    .ReadAll<MaterialComponent>()
    .NoneAll<HiddenTag>()
    .commit();

// 提交到渲染器
render_query.foreach([&renderer](
    const TransformComponent& transform,
    const MeshComponent& mesh,
    const MaterialComponent& material) {
    
    renderer.draw(mesh.handle, material.handle, 
        transform.to_matrix());
});
```

### 物理系统集成

```cpp
// 同步物理结果
auto physics_query = storage->new_query()
    .ReadWriteAll<PositionComponent>()
    .ReadWriteAll<VelocityComponent>()
    .ReadAll<RigidBodyComponent>()
    .commit();

physics_query.foreach([&physics_world](
    PositionComponent& pos,
    VelocityComponent& vel,
    const RigidBodyComponent& body) {
    
    // 从物理引擎获取结果
    auto physics_pos = physics_world.get_position(body.handle);
    auto physics_vel = physics_world.get_velocity(body.handle);
    
    // 更新 ECS 数据
    pos = physics_pos;
    vel = physics_vel;
});
```

## 总结

Sugoi ECS 是一个现代化、高性能的 Entity Component System 实现，专为数据密集型游戏设计。通过合理使用其特性，可以构建出缓存友好、易于并行化、可维护的游戏逻辑架构。记住 ECS 的核心理念：组件是数据，系统是逻辑，实体是标识符。