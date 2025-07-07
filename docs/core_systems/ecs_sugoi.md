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
```

## 查询系统

### Query Builder API

使用类型安全的 API 构建查询：

```cpp
// 创建查询
auto query = storage->new_query()
    .ReadWriteAll<PositionComponent>()      // 读写位置
    .ReadAll<VelocityComponent>()           // 只读速度
    .commit();
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

## 性能优化

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

## 总结

Sugoi ECS 是一个现代化、高性能的 Entity Component System 实现，专为数据密集型游戏设计。通过合理使用其特性，可以构建出缓存友好、易于并行化、可维护的游戏逻辑架构。记住 ECS 的核心理念：组件是数据，系统是逻辑，实体是标识符。