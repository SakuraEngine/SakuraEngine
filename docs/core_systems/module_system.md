# SakuraEngine 模块系统

## 概述

SakuraEngine 的模块系统提供了灵活的代码组织和加载机制，支持静态链接、动态加载和热重载三种模式。系统通过依赖图管理模块间关系，并提供子系统机制支持模块内的功能扩展。

## 核心概念

### 模块类型

```cpp
// 基础模块接口
class IModule {
public:
    // 生命周期回调
    virtual void on_load(ModuleManager* manager) = 0;
    virtual void on_unload() = 0;
    
    // 主模块执行入口
    virtual int main_module_exec(int argc, char** argv) { return 0; }
    
    // 元数据访问
    virtual const ModuleMetaData* get_meta_data() = 0;
};

// 静态模块（编译时链接）
class IStaticModule : public IModule {
    // 直接链接到主程序
};

// 动态模块（运行时加载）
class IDynamicModule : public IModule {
    // 从 DLL/SO 文件加载
    SharedLibrary* sharedLib;
};

// 热重载模块
class IHotfixModule : public IDynamicModule {
    // 额外的热重载支持
    virtual void on_reload_begin(HotfixState* state) = 0;
    virtual void on_reload_finish(HotfixState* state) = 0;
};
```

### 模块依赖

```
┌─────────────┐     ┌─────────────┐
│   ModuleA   │────>│   ModuleB   │
└─────────────┘     └─────────────┘
       │                    │
       │                    ▼
       │            ┌─────────────┐
       └──────────>│   ModuleC   │
                    └─────────────┘
```

## 创建模块

### 定义静态模块

```cpp
// MyModule.h
class MyModule : public IStaticModule {
public:
    virtual void on_load(ModuleManager* manager) override;
    virtual void on_unload() override;
    virtual const ModuleMetaData* get_meta_data() override;
    
    // 模块功能
    void do_something();
    
    // 单例访问
    static MyModule* Get();
};

// MyModule.cpp
#include "MyModule.h"

// 实现静态模块
IMPLEMENT_STATIC_MODULE(MyModule, my_module);

// 定义模块元数据
const char8_t* module_meta = u8R"(
{
    "name": "MyModule",
    "version": "1.0.0",
    "dependencies": [
        {"name": "SkrCore", "version": "1.0.0"}
    ],
    "linking": "static"
})";

void MyModule::on_load(ModuleManager* manager) {
    SKR_LOG_INFO(u8"MyModule loaded!");
    
    // 初始化模块资源
    initialize_resources();
    
    // 获取依赖模块
    auto core = manager->get_module<SkrCoreModule>();
}

void MyModule::on_unload() {
    SKR_LOG_INFO(u8"MyModule unloading...");
    
    // 清理资源
    cleanup_resources();
}

const ModuleMetaData* MyModule::get_meta_data() {
    static ModuleMetaData meta;
    static bool initialized = false;
    
    if (!initialized) {
        parse_module_meta(module_meta, &meta);
        initialized = true;
    }
    
    return &meta;
}

MyModule* MyModule::Get() {
    return ModuleManager::Get()->get_module<MyModule>();
}
```

### 定义动态模块

```cpp
// DynamicModule.h
class DynamicModule : public IDynamicModule {
public:
    virtual void on_load(ModuleManager* manager) override;
    virtual void on_unload() override;
    
    // 导出的功能
    SKR_MODULE_API void exported_function();
};

// DynamicModule.cpp
IMPLEMENT_DYNAMIC_MODULE(DynamicModule, dynamic_module);

const char8_t* module_meta = u8R"(
{
    "name": "DynamicModule",
    "version": "1.0.0",
    "dependencies": [],
    "linking": "shared"
})";

void DynamicModule::on_load(ModuleManager* manager) {
    // 动态模块初始化
}
```

### 定义热重载模块

```cpp
// HotfixModule.h
class HotfixModule : public IHotfixModule {
    struct State {
        int counter;
        std::string data;
    };
    
public:
    virtual void on_load(ModuleManager* manager) override;
    virtual void on_unload() override;
    virtual void on_reload_begin(HotfixState* state) override;
    virtual void on_reload_finish(HotfixState* state) override;
    
private:
    State internal_state;
};

// HotfixModule.cpp
void HotfixModule::on_reload_begin(HotfixState* state) {
    // 保存状态
    auto* saved = new State(internal_state);
    state->state = saved;
    
    SKR_LOG_INFO(u8"Saving state: counter=%d", saved->counter);
}

void HotfixModule::on_reload_finish(HotfixState* state) {
    // 恢复状态
    if (state->state) {
        auto* saved = static_cast<State*>(state->state);
        internal_state = *saved;
        delete saved;
        
        SKR_LOG_INFO(u8"Restored state: counter=%d", internal_state.counter);
    }
}
```

## 子系统机制

### 定义子系统

```cpp
// 子系统基类
struct MySubsystem : public ModuleSubsystem {
    // 子系统生命周期
    virtual void Initialize() override {
        SKR_LOG_INFO(u8"MySubsystem initialized");
        // 初始化子系统
    }
    
    virtual void Finalize() override {
        SKR_LOG_INFO(u8"MySubsystem finalized");
        // 清理子系统
    }
    
    // 热重载支持
    virtual void BeginReload() override {
        // 保存子系统状态
    }
    
    virtual void EndReload() override {
        // 恢复子系统状态
    }
    
    // 子系统功能
    void process_data();
};

// 注册子系统
SKR_MODULE_SUBSYSTEM(MySubsystem, MyModule);

// 在模块中访问子系统
void MyModule::on_load(ModuleManager* manager) {
    // 子系统会自动创建和初始化
    
    // 访问子系统
    for (auto& subsystem : subsystems) {
        if (auto* my_sub = dynamic_cast<MySubsystem*>(subsystem.get())) {
            my_sub->process_data();
        }
    }
}
```

### 子系统工厂

```cpp
// 自定义子系统创建
class CustomSubsystemFactory {
public:
    static ModuleSubsystemBase* create_subsystem(const char* name) {
        if (strcmp(name, "CustomSubsystem") == 0) {
            return new CustomSubsystem();
        }
        return nullptr;
    }
};

// 注册工厂
void register_subsystem_factory() {
    ModuleManager::Get()->register_subsystem_factory(
        "CustomSubsystem", 
        &CustomSubsystemFactory::create_subsystem
    );
}
```

## 模块管理器

### 初始化和销毁

```cpp
// 创建模块管理器
auto module_manager = skr::make_unique<ModuleManager>();

// 初始化根模块
bool success = module_manager->make_module_graph("MyRootModule");
if (!success) {
    SKR_LOG_ERROR(u8"Failed to create module graph");
    return;
}

// 初始化整个模块图
module_manager->init_module_graph();

// 运行主模块
int result = module_manager->main_module_exec(argc, argv);

// 销毁模块图
module_manager->destroy_module_graph();
```

### 模块访问

```cpp
// 获取模块实例
auto my_module = module_manager->get_module<MyModule>();
if (my_module) {
    my_module->do_something();
}

// 通过名称获取模块
IModule* module = module_manager->get_module_by_name("MyModule");

// 遍历所有模块
module_manager->foreach_module([](IModule* module) {
    SKR_LOG_INFO(u8"Module: %s", module->get_meta_data()->name);
});
```

## 热重载

### 触发热重载

```cpp
// 监视模块文件变化
class ModuleWatcher {
    filesystem_watcher watcher;
    ModuleManager* manager;
    
public:
    void watch_module(const char* module_name, const char* dll_path) {
        watcher.watch(dll_path, [=](const FileEvent& event) {
            if (event.type == FileEvent::Modified) {
                trigger_reload(module_name);
            }
        });
    }
    
    void trigger_reload(const char* module_name) {
        SKR_LOG_INFO(u8"Reloading module: %s", module_name);
        
        // 执行热重载
        manager->reload_module(module_name);
    }
};
```

### 处理热重载状态

```cpp
// 复杂状态的保存和恢复
class GameModule : public IHotfixModule {
    // 需要保存的运行时状态
    struct RuntimeState {
        std::vector<Entity> entities;
        std::unordered_map<uint32_t, Component> components;
        GameSettings settings;
    };
    
    void on_reload_begin(HotfixState* state) override {
        // 序列化状态到内存
        auto saved = new RuntimeState();
        
        // 深拷贝实体
        saved->entities = entities;
        
        // 保存组件数据
        saved->components = components;
        
        // 保存设置
        saved->settings = settings;
        
        state->state = saved;
    }
    
    void on_reload_finish(HotfixState* state) override {
        if (!state->state) return;
        
        auto saved = static_cast<RuntimeState*>(state->state);
        
        // 恢复状态
        entities = std::move(saved->entities);
        components = std::move(saved->components);
        settings = saved->settings;
        
        // 重新初始化必要的资源
        reinitialize_resources();
        
        delete saved;
    }
};
```

## 高级特性

### 延迟加载

```cpp
// 按需加载模块
class LazyModuleLoader {
    ModuleManager* manager;
    std::unordered_set<std::string> loaded_modules;
    
public:
    template<typename T>
    T* load_module_lazy() {
        const char* module_name = T::get_module_name();
        
        if (loaded_modules.find(module_name) == loaded_modules.end()) {
            // 首次访问，加载模块
            manager->load_module(module_name);
            loaded_modules.insert(module_name);
        }
        
        return manager->get_module<T>();
    }
};
```

### 模块间通信

```cpp
// 事件系统
class ModuleEventBus {
    using EventHandler = std::function<void(const void*)>;
    std::unordered_map<std::type_index, std::vector<EventHandler>> handlers;
    
public:
    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> handler) {
        handlers[std::type_index(typeid(EventType))].push_back(
            [handler](const void* data) {
                handler(*static_cast<const EventType*>(data));
            }
        );
    }
    
    template<typename EventType>
    void publish(const EventType& event) {
        auto it = handlers.find(std::type_index(typeid(EventType)));
        if (it != handlers.end()) {
            for (auto& handler : it->second) {
                handler(&event);
            }
        }
    }
};

// 使用事件通信
void ModuleA::on_load(ModuleManager* manager) {
    auto event_bus = manager->get_event_bus();
    
    // 订阅事件
    event_bus->subscribe<PlayerSpawnEvent>(
        [this](const PlayerSpawnEvent& e) {
            on_player_spawn(e.player_id, e.position);
        }
    );
}

void ModuleB::spawn_player(uint32_t id, float3 pos) {
    // 发布事件
    auto event_bus = ModuleManager::Get()->get_event_bus();
    event_bus->publish(PlayerSpawnEvent{id, pos});
}
```

### 模块配置

```cpp
// 模块配置系统
class ModuleConfig {
    json config_data;
    
public:
    void load_from_file(const char* path) {
        std::ifstream file(path);
        file >> config_data;
    }
    
    template<typename T>
    T get(const std::string& key, const T& default_value = T{}) {
        if (config_data.contains(key)) {
            return config_data[key].get<T>();
        }
        return default_value;
    }
};

// 在模块中使用配置
void MyModule::on_load(ModuleManager* manager) {
    ModuleConfig config;
    config.load_from_file("configs/my_module.json");
    
    int max_connections = config.get<int>("max_connections", 100);
    std::string server_url = config.get<std::string>("server_url");
}
```

## 最佳实践

### DO - 推荐做法

1. **明确模块职责**
   ```cpp
   // 好：单一职责模块
   class RenderModule { /* 渲染相关 */ };
   class PhysicsModule { /* 物理相关 */ };
   
   // 避免：巨型模块
   class EverythingModule { /* 所有功能 */ };
   ```

2. **合理使用依赖**
   ```cpp
   // 显式声明依赖
   "dependencies": [
       {"name": "RequiredModule", "version": "1.0.0"}
   ]
   ```

3. **支持热重载**
   ```cpp
   // 设计可序列化的状态
   struct ModuleState {
       // POD 类型或可序列化类型
   };
   ```

### DON'T - 避免做法

1. **避免循环依赖**
   ```cpp
   // 错误：A 依赖 B，B 依赖 A
   // 使用接口或事件系统解耦
   ```

2. **不要在构造函数中访问其他模块**
   ```cpp
   // 错误：构造时其他模块可能未初始化
   MyModule() {
       OtherModule::Get()->do_something(); // 危险！
   }
   
   // 正确：在 on_load 中访问
   void on_load(ModuleManager* manager) {
       auto other = manager->get_module<OtherModule>();
   }
   ```

## 调试技巧

### 模块加载日志

```cpp
// 启用详细日志
module_manager->set_log_level(LogLevel::Debug);

// 自定义日志处理
module_manager->set_log_callback(
    [](LogLevel level, const char* message) {
        if (level >= LogLevel::Warning) {
            // 记录到文件
            log_to_file(message);
        }
    }
);
```

### 依赖图可视化

```cpp
// 导出依赖图
void export_dependency_graph(ModuleManager* manager) {
    std::ofstream file("module_deps.dot");
    file << "digraph ModuleDependencies {\n";
    
    manager->foreach_module([&](IModule* module) {
        auto meta = module->get_meta_data();
        
        for (auto& dep : meta->dependencies) {
            file << "  \"" << meta->name << "\" -> \"" 
                 << dep.name << "\";\n";
        }
    });
    
    file << "}\n";
}
```

## 总结

SakuraEngine 的模块系统提供了灵活而强大的代码组织机制。通过支持多种加载模式、自动依赖管理、子系统扩展和热重载功能，系统能够满足大型游戏项目的开发需求。合理使用模块系统可以提高代码的可维护性、可扩展性，并加速开发迭代。