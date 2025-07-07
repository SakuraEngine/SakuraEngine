# SakuraEngine 资源系统

## 概述

SakuraEngine 的资源系统是一个高性能、异步的资源管理框架，负责游戏中所有资源的加载、依赖管理、生命周期控制和内存优化。系统设计支持大规模开放世界游戏的资源需求，包括流式加载、热重载和智能依赖解析。

## 核心概念

### 资源标识

#### 资源句柄 (skr_resource_handle_t)

资源句柄是访问资源的核心接口，采用了巧妙的 union 设计：

```cpp
typedef union skr_resource_handle_t {
    // 未解析状态：存储 GUID
    skr_guid_t guid;
    
    // 已解析状态：存储资源记录指针
    struct {
        uintptr_t ptr;      // 指向 skr_resource_record_t
        uint32_t padding;   // 0 表示已解析
        uint32_t _padding;
    };
} skr_resource_handle_t;

// 检查句柄是否已解析
bool is_resolved(skr_resource_handle_t handle) {
    return handle.padding == 0;
}

// 获取资源记录
skr_resource_record_t* get_record(skr_resource_handle_t handle) {
    return is_resolved(handle) ? (skr_resource_record_t*)handle.ptr : nullptr;
}
```

#### 资源记录 (skr_resource_record_t)

资源记录存储资源的所有元数据和状态信息：

```cpp
struct skr_resource_record_t {
    // 资源数据指针
    void* resource;
    
    // 资源头信息
    skr_resource_header_t header;
    
    // 加载状态
    ESkrLoadingPhase loadingPhase;
    ESkrLoadingStatus loadingStatus;
    
    // 请求者追踪
    skr::FlatHashMap<skr_requester_t, uint32_t> requesters;
    
    // 依赖资源
    skr::Vector<skr_resource_handle_t> dependencies;
    
    // 回调列表
    skr::Vector<skr_resource_record_t::callback_t> callbacks;
};
```

### 资源类型系统

每种资源类型都有对应的工厂：

```cpp
struct SResourceFactory {
    // 资源类型 ID
    skr_type_id_t resourceType;
    
    // 是否支持异步反序列化
    bool AsyncSerdeLoadFactor;
    
    // 反序列化函数
    virtual skr_resource_record_t* Deserialize(
        skr_resource_record_t* record,
        skr_binary_reader_t* reader) = 0;
        
    // 安装/卸载函数
    virtual bool Install(skr_resource_record_t* record);
    virtual bool Uninstall(skr_resource_record_t* record);
};
```

### 请求者系统

资源系统追踪谁在使用资源：

```cpp
// 请求者类型
enum ESkrRequesterType : uint32_t {
    SKR_REQUESTER_ENTITY = 0,     // ECS 实体
    SKR_REQUESTER_DEPENDENCY = 1,  // 作为依赖
    SKR_REQUESTER_SYSTEM = 2,      // 系统请求
    SKR_REQUESTER_SCRIPT = 3,      // 脚本请求
    SKR_REQUESTER_CUSTOM = 1u << 31 // 自定义类型
};

// 请求者结构
struct skr_requester_t {
    union {
        sugoi_entity_t entity;
        skr_guid_t guid;
        uint64_t custom;
    };
    ESkrRequesterType type;
};
```

## 资源加载流程

### 加载阶段

资源加载经历以下阶段：

```cpp
enum ESkrLoadingPhase {
    SKR_LOADING_PHASE_NONE = 0,
    SKR_LOADING_PHASE_REQUEST_RESOURCE,     // 1. 请求资源
    SKR_LOADING_PHASE_IO,                   // 2. IO 操作
    SKR_LOADING_PHASE_DESER_RESOURCE,       // 3. 反序列化
    SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES, // 4. 等待依赖
    SKR_LOADING_PHASE_INSTALL_RESOURCE,     // 5. 安装资源
    SKR_LOADING_PHASE_FINISHED              // 6. 完成
};
```

### 同步加载

最简单的资源加载方式：

```cpp
// 加载纹理资源
auto texture_handle = skr_load_resource<TextureResource>(
    u8"textures/character_diffuse.texture");

// 等待加载完成
if (texture_handle.is_resolved()) {
    TextureResource* texture = texture_handle.get_resource<TextureResource>();
    // 使用纹理...
}
```

### 异步加载

使用回调进行异步加载：

```cpp
// 异步加载模型
auto model_handle = skr_load_resource_async<ModelResource>(
    u8"models/character.model",
    [](skr_resource_handle_t handle) {
        ModelResource* model = handle.get_resource<ModelResource>();
        SKR_LOG_INFO(u8"Model loaded: %s", model->name.c_str());
    });

// 或者使用 Future 模式
auto future = skr_load_resource_future<ModelResource>(
    u8"models/character.model");

// 在其他地方检查
if (future.is_ready()) {
    ModelResource* model = future.get();
}
```

### 批量加载

高效加载多个资源：

```cpp
// 准备批量加载请求
skr::Vector<skr_resource_request_t> requests;
requests.push_back({u8"texture1.texture", SKR_REQUESTER_SYSTEM});
requests.push_back({u8"texture2.texture", SKR_REQUESTER_SYSTEM});
requests.push_back({u8"model.model", SKR_REQUESTER_SYSTEM});

// 批量加载
skr_load_resources_batch(requests.data(), requests.size(),
    [](skr_resource_handle_t* handles, size_t count) {
        SKR_LOG_INFO(u8"Batch loaded %zu resources", count);
    });
```

## 依赖管理

### 自动依赖加载

资源可以声明依赖，系统会自动加载：

```cpp
// 材质资源定义
sreflect_struct("guid": "12345678-...")
struct MaterialResource {
    // 材质依赖的纹理
    skr_resource_handle_t diffuse_texture;
    skr_resource_handle_t normal_texture;
    skr_resource_handle_t metallic_texture;
    
    // 材质参数
    float metallic_factor;
    float roughness_factor;
};

// 加载材质时，所有纹理依赖会自动加载
auto material = skr_load_resource<MaterialResource>(
    u8"materials/metal.material");
```

### 依赖图查询

```cpp
// 获取资源的所有依赖
void print_dependencies(skr_resource_handle_t handle) {
    auto record = handle.get_record();
    if (!record) return;
    
    SKR_LOG_INFO(u8"Dependencies for %s:", 
        record->header.guid.to_string().c_str());
    
    for (auto& dep : record->dependencies) {
        auto dep_record = dep.get_record();
        if (dep_record) {
            SKR_LOG_INFO(u8"  - %s", 
                dep_record->header.guid.to_string().c_str());
        }
    }
}
```

## 生命周期管理

### 引用计数

资源使用引用计数管理生命周期：

```cpp
// 增加引用
skr_requester_t requester = {
    .entity = entity,
    .type = SKR_REQUESTER_ENTITY
};
skr_add_resource_ref(handle, requester);

// 释放引用
skr_release_resource_ref(handle, requester);

// 查询引用计数
uint32_t ref_count = skr_get_resource_ref_count(handle);
```

### 手动控制

```cpp
// 强制保持资源在内存中
skr_pin_resource(handle);

// 允许资源被卸载
skr_unpin_resource(handle);

// 立即卸载未使用的资源
skr_gc_resources();
```

## I/O 服务集成

资源系统与 I/O 服务深度集成，支持高性能异步加载：

### RAM Service

```cpp
// 获取 RAM 服务
IRAMService* ram_service = skr_get_ram_service();

// 创建 IO 请求
skr_ram_io_future_t future;
ram_service->request(path, &buffer, &future);

// 在资源系统中使用
struct AsyncLoadContext {
    skr_resource_record_t* record;
    skr_ram_io_future_t io_future;
    skr::Vector<uint8_t> buffer;
};
```

### 压缩支持

```cpp
// 资源可以是压缩的
struct CompressedResourceHeader {
    uint32_t uncompressed_size;
    uint32_t compressed_size;
    ECompressionType compression_type;
};

// 系统自动处理解压
auto texture = skr_load_resource<TextureResource>(
    u8"textures/large_texture.texture.lz4");
```

## 资源工厂

### 创建自定义资源类型

```cpp
// 1. 定义资源类型
sreflect_struct("guid": "87654321-...", "serialize": ["json", "bin"])
struct CustomResource {
    SkrString name;
    float value;
    skr::Vector<int> data;
};

// 2. 实现资源工厂
class CustomResourceFactory : public SResourceFactory {
public:
    CustomResourceFactory() {
        resourceType = type_id_of<CustomResource>::get();
        AsyncSerdeLoadFactor = true;
    }
    
    skr_resource_record_t* Deserialize(
        skr_resource_record_t* record,
        skr_binary_reader_t* reader) override {
        
        // 分配资源
        auto* resource = SkrNew<CustomResource>();
        record->resource = resource;
        
        // 反序列化
        skr_binary_read(reader, resource);
        
        return record;
    }
    
    bool Install(skr_resource_record_t* record) override {
        auto* resource = (CustomResource*)record->resource;
        // 安装后处理
        SKR_LOG_INFO(u8"Installing custom resource: %s", 
            resource->name.c_str());
        return true;
    }
    
    bool Uninstall(skr_resource_record_t* record) override {
        // 卸载前清理
        return true;
    }
};

// 3. 注册工厂
skr_register_resource_factory<CustomResourceFactory>();
```

## 高级特性

### 资源热重载

开发模式下支持资源热重载：

```cpp
// 启用热重载
skr_enable_hot_reload(true);

// 监听资源变更
skr_register_resource_change_callback(
    [](skr_resource_handle_t handle, EResourceChangeType change) {
        switch (change) {
            case RESOURCE_MODIFIED:
                SKR_LOG_INFO(u8"Resource modified, reloading...");
                break;
            case RESOURCE_DELETED:
                SKR_LOG_WARN(u8"Resource deleted!");
                break;
        }
    });

// 手动触发重载
skr_reload_resource(handle);
```

### 资源打包

```cpp
// 资源可以打包成 bundle
struct ResourceBundle {
    skr::Vector<skr_resource_entry_t> entries;
    skr::Vector<uint8_t> data;
};

// 加载 bundle 中的资源
auto bundle = skr_load_bundle(u8"levels/level1.bundle");
auto texture = skr_load_resource_from_bundle<TextureResource>(
    bundle, u8"textures/wall.texture");
```

### 内存预算

```cpp
// 设置资源内存预算
skr_set_resource_memory_budget(512 * 1024 * 1024); // 512MB

// 查询当前使用情况
ResourceMemoryStats stats;
skr_get_resource_memory_stats(&stats);
SKR_LOG_INFO(u8"Resource memory: %zuMB / %zuMB", 
    stats.used / 1024 / 1024,
    stats.budget / 1024 / 1024);

// 设置优先级
skr_set_resource_priority(handle, RESOURCE_PRIORITY_HIGH);
```

## 性能优化

### 预加载策略

```cpp
// 预加载关卡资源
class LevelPreloader {
    skr::Vector<skr_resource_handle_t> preloaded_resources;
    
public:
    void preload_level(const char8_t* level_name) {
        // 读取关卡资源清单
        auto manifest = load_level_manifest(level_name);
        
        // 批量预加载
        for (auto& entry : manifest.resources) {
            auto handle = skr_load_resource_async(
                entry.path.c_str(), 
                entry.type,
                SKR_REQUESTER_SYSTEM);
            preloaded_resources.push_back(handle);
        }
    }
    
    bool is_ready() {
        for (auto& handle : preloaded_resources) {
            if (!handle.is_resolved() || 
                !handle.get_record()->is_installed()) {
                return false;
            }
        }
        return true;
    }
};
```

### 流式加载

```cpp
// 基于距离的流式加载
class StreamingManager {
    struct StreamingRegion {
        float3 center;
        float radius;
        skr::Vector<skr_resource_handle_t> resources;
    };
    
    skr::Vector<StreamingRegion> regions;
    
public:
    void update(float3 player_position) {
        for (auto& region : regions) {
            float distance = length(region.center - player_position);
            
            if (distance < region.radius) {
                // 加载区域资源
                for (auto& handle : region.resources) {
                    if (!handle.is_resolved()) {
                        skr_load_resource_async(handle);
                    }
                }
            } else if (distance > region.radius * 2) {
                // 卸载远处资源
                for (auto& handle : region.resources) {
                    skr_release_resource_ref(handle, streaming_requester);
                }
            }
        }
    }
};
```

## 最佳实践

### DO - 推荐做法

1. **使用类型安全的接口**
   ```cpp
   // 好：类型安全
   auto texture = skr_load_resource<TextureResource>(path);
   
   // 避免：需要手动转换
   auto handle = skr_load_resource_raw(path, type_id);
   ```

2. **合理使用异步加载**
   ```cpp
   // 关键资源同步加载
   auto player_model = skr_load_resource<ModelResource>(
       u8"models/player.model");
   
   // 非关键资源异步加载
   skr_load_resource_async<TextureResource>(
       u8"textures/detail.texture", callback);
   ```

3. **批量操作**
   ```cpp
   // 批量加载相关资源
   skr::Vector<skr_resource_request_t> batch;
   // ... 填充批量请求
   skr_load_resources_batch(batch.data(), batch.size());
   ```

### DON'T - 避免做法

1. **避免阻塞等待**
   ```cpp
   // 不好：阻塞主线程
   while (!handle.is_resolved()) {
       std::this_thread::sleep_for(1ms);
   }
   
   // 好：使用回调或轮询
   if (handle.is_resolved()) {
       // 处理资源
   }
   ```

2. **不要忘记释放引用**
   ```cpp
   // 确保配对的引用计数
   skr_add_resource_ref(handle, requester);
   // ... 使用资源
   skr_release_resource_ref(handle, requester);
   ```

3. **避免循环依赖**
   ```cpp
   // 资源 A 依赖 B，B 依赖 A 会导致死锁
   // 使用弱引用或重新设计资源结构
   ```

## 调试工具

### 资源统计

```cpp
// 打印资源使用统计
void debug_resource_stats() {
    ResourceSystemStats stats;
    skr_get_resource_system_stats(&stats);
    
    SKR_LOG_INFO(u8"=== Resource System Stats ===");
    SKR_LOG_INFO(u8"Total resources: %u", stats.total_resources);
    SKR_LOG_INFO(u8"Loaded resources: %u", stats.loaded_resources);
    SKR_LOG_INFO(u8"Loading resources: %u", stats.loading_resources);
    SKR_LOG_INFO(u8"Memory usage: %.2fMB", 
        stats.memory_usage / 1024.0f / 1024.0f);
    
    // 按类型统计
    for (auto& type_stat : stats.type_stats) {
        SKR_LOG_INFO(u8"  %s: %u resources, %.2fMB",
            type_stat.type_name,
            type_stat.count,
            type_stat.memory / 1024.0f / 1024.0f);
    }
}
```

### 资源泄漏检测

```cpp
// 检测未释放的资源引用
void debug_check_leaks() {
    skr_foreach_resource([](skr_resource_record_t* record) {
        if (record->requesters.size() > 0) {
            SKR_LOG_WARN(u8"Resource %s has %zu active references:",
                record->header.guid.to_string().c_str(),
                record->requesters.size());
            
            for (auto& [requester, count] : record->requesters) {
                const char* type_name = get_requester_type_name(requester.type);
                SKR_LOG_WARN(u8"  - %s: %u refs", type_name, count);
            }
        }
    });
}
```

### 加载性能分析

```cpp
// 启用加载性能统计
skr_enable_resource_profiling(true);

// 获取加载时间统计
ResourceLoadingProfile profile;
skr_get_resource_loading_profile(&profile);

SKR_LOG_INFO(u8"Average load times:");
SKR_LOG_INFO(u8"  IO: %.2fms", profile.avg_io_time);
SKR_LOG_INFO(u8"  Deserialize: %.2fms", profile.avg_deser_time);
SKR_LOG_INFO(u8"  Install: %.2fms", profile.avg_install_time);
```

## 错误处理

### 加载失败处理

```cpp
// 注册错误回调
skr_set_resource_error_callback(
    [](skr_resource_handle_t handle, const char8_t* error) {
        SKR_LOG_ERROR(u8"Failed to load resource: %s", error);
        
        // 使用占位资源
        auto record = handle.get_record();
        if (record && record->header.type == type_id_of<TextureResource>::get()) {
            record->resource = get_placeholder_texture();
            record->loadingStatus = SKR_LOADING_STATUS_ERROR;
        }
    });

// 检查加载状态
if (handle.get_status() == SKR_LOADING_STATUS_ERROR) {
    // 处理错误情况
}
```

## 与其他系统集成

### ECS 集成

```cpp
// 资源组件
sreflect_struct("ecs.comp": true)
struct MeshResourceComponent {
    skr_resource_handle_t mesh;
};

// 在系统中使用
void MeshRenderSystem::update() {
    query.foreach([](const MeshResourceComponent& mesh_comp,
                    const TransformComponent& transform) {
        if (mesh_comp.mesh.is_resolved()) {
            auto* mesh = mesh_comp.mesh.get_resource<MeshResource>();
            render_mesh(mesh, transform);
        }
    });
}
```

### 渲染系统集成

```cpp
// 资源就绪检查
bool are_render_resources_ready(const RenderObject& obj) {
    return obj.mesh.is_resolved() &&
           obj.material.is_resolved() &&
           obj.material.get_resource<MaterialResource>()
               ->are_textures_ready();
}
```

## 总结

SakuraEngine 的资源系统提供了一个强大、灵活的资源管理解决方案。通过异步加载、智能依赖管理和生命周期控制，它能够支持从小型游戏到大型开放世界的各种需求。合理使用资源系统的各项特性，可以实现高效的内存使用和流畅的游戏体验。